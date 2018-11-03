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

#include <QDateTime>
#include <QLocale>
#include <QSqlQuery>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

#include "globals.h"
#include "node.h"
#include "utils.h"

#include "ddir.h"

DDir::DDir(qint64 t_id)
    : id(t_id)
{
}

bool DDir::loadFromDB()
{
    if (dbLoaded) return false;

    QSqlQuery query;
    if (!query.exec(QString("select diskid,parent,numitems,name,modtime,fowner,fgroup,qpermissions,accessdenied from directories where id = %1").arg(id))) return false;
    if (!query.next()) return false;

    diskID = query.value(0).toLongLong();
    parentDirID = query.value(1).toLongLong();
    numItems = query.value(2).toLongLong();
    dirName = query.value(3).toString();
    modtime = query.value(4).toLongLong();
    fOwner = query.value(5).toString();
    fGroup = query.value(6).toString();
    qPermissions = query.value(7).toLongLong();
    if (query.value(8).toInt() > 0) accessDenied = true;

    qdtLastModified.setSecsSinceEpoch(modtime);

    QList<QString> parents;
    if (parentDirID != 0) // is not a root directory
    {
        qint64 pparentDirID = parentDirID;
        while(true)
        {
            query.exec(QString("select diskid,parent,name from directories where id = %1").arg(pparentDirID));
            query.next();
            pparentDirID = query.value(1).toLongLong();
            diskID = query.value(0).toLongLong();
            if (pparentDirID == 0) break;
            parents.prepend(query.value(2).toString());
        }
    }

    diskPath.reserve(1024);

    foreach(QString q, parents)
    {
        diskPath.append("/");
        diskPath.append(q);
    }

    if (!query.exec(QString("select catid,name,catpath from disks where id = %1").arg(diskID))) return false;
    if (!query.next()) return false;
    catID = query.value(0).toLongLong();
    diskName = query.value(1).toString();

    containerURL = query.value(2).toString();
    containerURL += diskPath;
    fullURL = containerURL + "/" + dirName;

    if (catID)
    {
        if (!query.exec(QString("select name from catalogues where id = %1").arg(catID))) return false;
        if (!query.next()) return false;
        catName = query.value(0).toString();
    }

    if (parents.size() == 0) diskPath = "/";
    if (dirName.isEmpty()) dirName = "/";

    dbLoaded = true;
    return true;
}

void DDir::loadFromFileSystem()
{
    if (fsLoaded) return;

    qdir.setPath(fullURL);
    qfiContainer = QFileInfo(containerURL);

    fsLoaded = true;
}


const QString &DDir::getPermissionsText()
{
    if (permissionsText.isEmpty())
    {
        permissionsText = qPermissionsToText(qPermissions);
    }
    return permissionsText;
}

bool DDir::isReachable()
{
    if (!fsLoaded) loadFromFileSystem();
    return qdir.exists();
}

void DDir::osOpen()
{
    if (!isReachable()) return;
    if (!QDesktopServices::openUrl(QUrl(QString("file://") + fullURL)))
        Utils::errorMessageBox(NULL, "O/S open directory failed");
}

const QString& DDir::getLastModifiedFullText()
{
    if (lastModifiedFullText.isEmpty())
    {
        lastModifiedFullText = qdtLastModified.toString("dddd, d MMMM yyyy hh:mm:ss t");
    }
    return lastModifiedFullText;
}

const QString& DDir::getDirStats()
{
    if (dirStats.isEmpty()) dirStats = Node::dirStats(id);
    return dirStats;
}

const QString &DDir::getSubContents()
{
    if (dirSubContents.isEmpty())
    {
        qint64 numDirectories = 0;
        qint64 numFiles = 0;
        qint64 totalSize = 0;
        recurse(id, numDirectories, numFiles, totalSize);

        dirSubContents = QString::number(numDirectories);
        if (numDirectories == 1)
            dirSubContents += " directory, ";
        else
            dirSubContents += " directories, ";

        dirSubContents += QString::number(numFiles);
        if (numFiles == 1)
            dirSubContents += " file (";
        else
            dirSubContents += " files (";

        dirSubContents += fileSizeToHR(totalSize);
        dirSubContents += ")";
    }

    return dirSubContents;
}

void DDir::recurse(qint64 dirID, qint64& numDirectories, qint64& numFiles, qint64& totalSize) const
{
    bool b1, b2, b3, b4, b5;
    QSqlQuery query;
    b1 = query.exec(QString("select count(*) from files where dirid = %1").arg(dirID));
    b2 = query.next();
    numFiles += query.value(0).toLongLong();

    b3 = query.exec(QString("select sum(size) from files where dirid = %1").arg(dirID));
    b4 = query.next();
    totalSize += query.value(0).toLongLong();

    b5 = query.exec(QString("select id from directories where parent = %1").arg(dirID));

    if (!b1 || !b2 || !b3 || !b4 || !b5)
    {
        Utils::errorMessageBox(NULL, "Database error");
        return;
    }

    while(query.next())
    {
        ++numDirectories;
        recurse(query.value(0).toLongLong(), numDirectories, numFiles, totalSize);
    }
}
