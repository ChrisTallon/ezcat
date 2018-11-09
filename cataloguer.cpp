/*
 * This file is part of EZ Cat.
 * Copyright (C) 2018 Chris Tallon
 *
 * This program is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <blkid/blkid.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>

#include "globals.h"
#include "nodedisk.h"

#include "cataloguer.h"

Cataloguer::Cataloguer(qint64 t_catID, const QString& t_newDiskName, const QString& t_newPath)
    : catID(t_catID), newDiskName(t_newDiskName), newPath(t_newPath), disk(NULL)
{
}

Cataloguer::~Cataloguer()
{
}

void Cataloguer::updateMode(NodeDisk* _disk)
{
    disk = _disk;
}

void Cataloguer::abort()
{
    abortNow = true;
}

void Cataloguer::go()
{
    try
    {
        // Get a secondary database connection
        cdb = new DB();
        if (!cdb->initLib("cataloguer")) throw 5;
        if (!cdb->openDB()) throw 6;

        QSqlQuery updateDiskQuery(cdb->getqdb());
        if (!updateDiskQuery.prepare("update disks set catid = :catid, name = :name, catpath = :catpath, cattime = :cattime, "
                                     "devname = :devname, fslabel = :fslabel, fstype = :fstype, fssize = :fssize, "
                                     "fsfree = :fsfree, isroot = :isroot, uuid = :uuid where id = :id")) throw 10;

        numItemsQuery = new QSqlQuery(cdb->getqdb());
        if (!numItemsQuery->prepare("update directories set numitems = :numitems where id = :dirid")) throw 20;

        dirQuery = new QSqlQuery(cdb->getqdb());
        if (!dirQuery->prepare("insert into directories (diskid, parent, name, modtime, fowner, fgroup, qpermissions, accessdenied) "
                                      "values (:diskid, :parent, :name, :modtime, :fowner, :fgroup, :qpermissions, :accessdenied)")) throw 30;

        fileQuery = new QSqlQuery(cdb->getqdb());
        if (!fileQuery->prepare("insert into files (dirid, name, size, type, modtime, fowner, fgroup, qpermissions) "
                                      "values (:dirid, :name, :size, :type, :modtime, :fowner, :fgroup, :qpermissions)")) throw 40;

        QSqlQuery otherQueries(cdb->getqdb());

        if (!cdb->startTransaction())                                      throw 50;

        if (!otherQueries.exec("drop index directories_diskid_idx"))       throw 60;
        if (!otherQueries.exec("drop index directories_parent_idx"))       throw 70;
        if (!otherQueries.exec("drop index files_dirid_idx"))              throw 80;
        if (!otherQueries.exec("drop index directories_names_idx"))        throw 90;

        QDir root(newPath);
        rootStorageInfo = QStorageInfo(root);

        int isRoot = 0;
        if (newPath == rootStorageInfo.rootPath()) isRoot = 1;

        QString blkid;
        blkid_cache bc;
        if(!blkid_get_cache(&bc, NULL))
        {
            char* tag = blkid_get_tag_value(bc, "UUID", qPrintable(rootStorageInfo.device()));
            blkid = tag;
        }
        else
        {
            qDebug() << "libblkid: blkid_get_cache fail";
        }

        if (disk) // Updating a disk
        {
            if (!disk->removeContentsFromDBNT(otherQueries)) throw 100;

            qint64 timeNow = QDateTime::currentDateTime().toSecsSinceEpoch();

            updateDiskQuery.bindValue(":catid", catID);
            updateDiskQuery.bindValue(":name", newDiskName);
            updateDiskQuery.bindValue(":catpath", newPath);
            updateDiskQuery.bindValue(":cattime", timeNow);
            updateDiskQuery.bindValue(":devname", rootStorageInfo.device());
            updateDiskQuery.bindValue(":fslabel", rootStorageInfo.name());
            updateDiskQuery.bindValue(":fstype", rootStorageInfo.fileSystemType());
            updateDiskQuery.bindValue(":fssize", rootStorageInfo.bytesTotal());
            updateDiskQuery.bindValue(":fsfree", rootStorageInfo.bytesFree());
            updateDiskQuery.bindValue(":isroot", isRoot);
            updateDiskQuery.bindValue(":uuid", blkid);
            updateDiskQuery.bindValue(":id", disk->getID());
            if (!updateDiskQuery.exec()) throw 110;

            disk->update(catID, newDiskName, newPath, timeNow, rootStorageInfo.device(), rootStorageInfo.name(),
                         rootStorageInfo.fileSystemType(), rootStorageInfo.bytesTotal(), rootStorageInfo.bytesFree(), isRoot, blkid);
        }
        else
        {
            disk = NodeDisk::createDisk(otherQueries, catID, newDiskName, newPath, rootStorageInfo.device(), rootStorageInfo.name(),
                                        rootStorageInfo.fileSystemType(), rootStorageInfo.bytesTotal(), rootStorageInfo.bytesFree(), isRoot, blkid);
            if (!disk) throw 120;
        }

        // Make a root directory
        QFileInfo rootDirInfo(newPath);
        dirQuery->bindValue(":diskid", disk->getID());
        dirQuery->bindValue(":parent", 0);
        dirQuery->bindValue(":name", QVariant());
        dirQuery->bindValue(":modtime", rootDirInfo.lastModified().toSecsSinceEpoch());
        dirQuery->bindValue(":fowner", rootDirInfo.owner());
        dirQuery->bindValue(":fgroup", rootDirInfo.group());
        dirQuery->bindValue(":qpermissions", static_cast<int>(rootDirInfo.permissions()));
        dirQuery->bindValue(":accessdenied", 0);
        if (!dirQuery->exec()) throw 130;
        if (!disk->loadRootDirID(otherQueries)) throw 140;
        ++numObjects;

        recurse(root, disk->getRootDirID());
        emit numObjectsFound(numObjects);

        emit reindexing();

        if (!otherQueries.exec("create index directories_diskid_idx on directories(diskid)"))               throw 250;
        if (!otherQueries.exec("create index directories_parent_idx on directories(parent)"))               throw 260;
        if (!otherQueries.exec("create index files_dirid_idx on files(dirid)"))                             throw 270;
        if (!otherQueries.exec("create index directories_names_idx on directories(name collate nocase)"))   throw 280;

        if (!cdb->commitTransaction()) throw 290;
        cdb->closeDB();

        delete fileQuery;
        delete dirQuery;
        delete numItemsQuery;
        delete cdb;

        emit finished(disk);
    }
    catch (int e)
    {
        savedError = e;
        qDebug() << "Cataloguer error: " << e;
        if      (e == 5) qDebug() << "Failed to get private DB connection";
        else if (e == 6) qDebug() << "Open DB failed";
        else if (e == 10) qDebug() << "Update disk query prepare failed";
        else if (e == 20) qDebug() << "NumItems query prepare failed";
        else if (e == 30) qDebug() << "Directories query prepare failed";
        else if (e == 40) qDebug() << "Files query prepare failed";
        else if (e == 50) qDebug() << "Failed to start transaction";
        else if (e == 60) qDebug() << "Drop index query A failed";
        else if (e == 70) qDebug() << "Drop index query B failed";
        else if (e == 80) qDebug() << "Drop index query C failed";
        else if (e == 90) qDebug() << "Drop index query D failed";
        else if (e == 100) qDebug() << "removeContentsFromDBNT failed";
        else if (e == 110) qDebug() << "Update disk query exec failed";
        else if (e == 120) qDebug() << "NodeDisk::createDisk failed";
        else if (e == 130) qDebug() << "Root directory query exec failed";
        else if (e == 140) qDebug() << "Disk::loadRootDirID failed";
        else if (e == 200) qDebug() << "Recurse: NumItems query exec failed";
        else if (e == 210) qDebug() << "Recurse: Cataloguing aborted";
        else if (e == 220) qDebug() << "Recurse: Files query exec failed";
        else if (e == 230) qDebug() << "Recurse: Directories query exec failed";
        else if (e == 240) qDebug() << "Recurse: Files query (other) exec failed";
        else if (e == 250) qDebug() << "Reindexing query A failed";
        else if (e == 260) qDebug() << "Reindexing query B failed";
        else if (e == 270) qDebug() << "Reindexing query C failed";
        else if (e == 280) qDebug() << "Reindexing query D failed";
        else if (e == 290) qDebug() << "Commit transaction failed";

        switch(e)
        {
        case 290:
        case 280:
        case 270:
        case 260:
        case 250:
        case 240:
        case 230:
        case 220:
        case 210:
        case 200:
            emit numObjectsFound(numObjects);
            [[fallthrough]];
        case 140:
        case 130:
        case 120:
        case 110:
        case 100:
        case 90:
        case 80:
        case 70:
        case 60:
            cdb->rollbackTransaction();
            [[fallthrough]];
        case 50:
        case 40:
            delete fileQuery;
            [[fallthrough]];
        case 30:
            delete dirQuery;
            [[fallthrough]];
        case 20:
            delete numItemsQuery;
            [[fallthrough]];
        case 10:
            cdb->closeDB();
            [[fallthrough]];
        case 6:
        case 5:
            delete cdb;
            emit finished(NULL);
        }
    }
}

void Cataloguer::recurse(const QDir& dir, qint64 dirid) // throws int
{
    subCheckStorageInfo.setPath(dir.absolutePath());
    if (subCheckStorageInfo.device() != rootStorageInfo.device())
    {
        qDebug() << "DIFFERENT STORAGE INFO, SKIPPING";
        qDebug() << subCheckStorageInfo.bytesTotal();
        qDebug() << subCheckStorageInfo.bytesFree();
        qDebug() << subCheckStorageInfo.device();
        qDebug() << subCheckStorageInfo.displayName();
        qDebug() << subCheckStorageInfo.fileSystemType();
        qDebug() << subCheckStorageInfo.name();
        qDebug() << subCheckStorageInfo.rootPath();
        qDebug() << "return";
        return;
    }

    // get all entries in dir - insert all these into db. Foreach child dir, recurse

    QFileInfoList ql = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    totalDirs += ql.size();

    numItemsQuery->bindValue(":numitems", ql.size());
    numItemsQuery->bindValue(":dirid", dirid);
    if (!numItemsQuery->exec()) throw 200;

    foreach(QFileInfo info, ql)
    {
        if (abortNow) throw 210;

        if (info.isSymLink() || info.isFile())
        {
            char type;
            if (info.isSymLink()) type = TYPE_SYMLINK;
            else type = TYPE_FILE;

            qint64 size;
            if (info.isSymLink()) size = 0;
            else size = info.size();

            fileQuery->bindValue(":dirid", dirid);
            fileQuery->bindValue(":name", info.fileName());
            fileQuery->bindValue(":size", size);
            fileQuery->bindValue(":type", type);
            fileQuery->bindValue(":modtime", info.lastModified().toSecsSinceEpoch());
            fileQuery->bindValue(":fowner", info.owner());
            fileQuery->bindValue(":fgroup", info.group());
            fileQuery->bindValue(":qpermissions", static_cast<int>(info.permissions()));

            if (!fileQuery->exec()) throw 220;
            if (++numObjects % 1000 == 0) emit numObjectsFound(numObjects);
        }
        else if (info.isDir())
        {
            int accessDenied = 0;
            if (!info.isReadable())
            {
                accessDenied = 1;
                accessDeniedPaths.append(info.absoluteFilePath());
            }

            dirQuery->bindValue(":diskid", disk->getID());
            dirQuery->bindValue(":parent", dirid);
            dirQuery->bindValue(":name", info.fileName());
            dirQuery->bindValue(":modtime", info.lastModified().toSecsSinceEpoch());
            dirQuery->bindValue(":fowner", info.owner());
            dirQuery->bindValue(":fgroup", info.group());
            dirQuery->bindValue(":qpermissions", static_cast<int>(info.permissions()));
            dirQuery->bindValue(":accessdenied", accessDenied);

            if (!dirQuery->exec()) throw 230;
            if (++numObjects % 1000 == 0) emit numObjectsFound(numObjects);

            qint64 newDirID = dirQuery->lastInsertId().toLongLong();
            QDir childDir(info.absoluteFilePath());
            recurse(childDir, newDirID);
        }
        else // pipes, devices ...
        {
            fileQuery->bindValue(":dirid", dirid);
            fileQuery->bindValue(":name", info.fileName());
            fileQuery->bindValue(":size", info.size());
            fileQuery->bindValue(":type", TYPE_OTHERFILEUNKNOWN);
            fileQuery->bindValue(":modtime", info.lastModified().toSecsSinceEpoch());
            fileQuery->bindValue(":fowner", info.owner());
            fileQuery->bindValue(":fgroup", info.group());
            fileQuery->bindValue(":qpermissions", static_cast<int>(info.permissions()));

            if (!fileQuery->exec()) throw 240;
            if (++numObjects % 1000 == 0) emit numObjectsFound(numObjects);
        }
    }
}
