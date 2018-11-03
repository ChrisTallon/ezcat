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

#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QProcess>
#include <QSqlTableModel>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

#include "globals.h"
#include "db.h"
#include "nodedir.h"
#include "utils.h"

#include "nodedisk.h"

NodeDisk::NodeDisk(qint64 t_id, qint64 t_catID, const QString& t_name, const QString& t_catPath,
     qint64 t_catTime, const QString& t_deviceName, const QString& t_fsLabel,
     const QString& t_fsType, qint64 t_fsSize, qint64 t_fsFree, int t_isRoot,
     const QString& t_mountCommand, const QString& t_unmountCommand, const QString& t_uuid)
    : Node(TYPE_DISK, t_id, t_name),
      catID(t_catID), catPath(t_catPath), catTime(t_catTime), deviceName(t_deviceName),
      fsLabel(t_fsLabel), fsType(t_fsType), fsSize(t_fsSize), fsFree(t_fsFree), isRoot(t_isRoot),
      mountCommand(t_mountCommand), unmountCommand(t_unmountCommand), uuid(t_uuid)
{}

bool NodeDisk::loadRootDirID()
{
    QSqlQuery getRootDirQuery;
    if (!getRootDirQuery.exec(QString("select id from directories where diskid = %1 and parent = 0").arg(id))) return false;
    if (!getRootDirQuery.next()) return false;
    rootDirID = getRootDirQuery.value(0).toLongLong();
    return true;
}

bool NodeDisk::moveToCatalogue(qint64 newCat)
{
    QSqlQuery query;
    if (!query.exec(QString("update disks set catid = %1 where id = %2").arg(newCat).arg(id)))
    {
        Utils::errorMessageBox(NULL, "Database Error:\ncmoveToCatalogue: Query fail");
        return false;
    }
    catID = newCat;
    return true;
}

bool NodeDisk::rename(const QString& _newName)
{
    QString newName = _newName.trimmed();
    if (newName.isEmpty()) return false;

    if (!dbman.startTransaction()) { Utils::errorMessageBox(NULL, "Disk::rename: Transaction start error"); return false; }

    QSqlQuery query;
    query.prepare("update disks set name = :newname where id = :existingid");
    query.bindValue(":newname", newName);
    query.bindValue(":existingid", id);
    if (query.exec())
    {
        if (dbman.commitTransaction())
        {
            name = newName;
            return true;
        }
        return false;
    }
    else
    {
        dbman.rollbackTransaction();
        return false;
    }
}

void NodeDisk::setCommands(const QString& newMountCommand, const QString& newUnmountCommand)
{
    QSqlQuery query;
    query.prepare("update disks set mountcmd = :mcom, umountcmd = :umcom where id = :id");
    query.bindValue(":mcom", newMountCommand);
    query.bindValue(":umcom", newUnmountCommand);
    query.bindValue(":id", id);

    if (query.exec())
    {
        mountCommand = newMountCommand;
        unmountCommand = newUnmountCommand;
    }
}

bool NodeDisk::hasMountCommand() const
{
    return !mountCommand.isEmpty();
}

bool NodeDisk::hasUnmountCommand() const
{
    return !unmountCommand.isEmpty();
}

bool NodeDisk::catPathLocationEmpty() const
{
    // returns true if catPath exists, is a dir, and is empty

    QFileInfo qfi(catPath);
    if (qfi.isDir())
    {
        QDir qd(catPath);
        if (qd.exists() && qd.isEmpty()) return true;
    }
    return false;
}

bool NodeDisk::catPathLocationFull() const
{
    // returns true if catPath exists and is a non-empty dir

    QFileInfo qfi(catPath);
    if (qfi.isDir())
    {
        QDir qd(catPath);
        if (qd.exists() && !qd.isEmpty()) return true;
    }
    return false;
}

bool NodeDisk::mount()
{
    QString program("/bin/sh");
    QStringList arguments;
    arguments << "-c" << mountCommand;
    return startProcess(program, arguments);
}

bool NodeDisk::unmount()
{
    QString program("/bin/sh");
    QStringList arguments;
    arguments << "-c" << unmountCommand;
    return startProcess(program, arguments);
}

bool NodeDisk::startProcess(const QString& program, const QStringList& arguments)
{
    bool retval = false;
    QProcess process;
    process.start(program, arguments);
    process.waitForFinished();

    if ((process.exitStatus() == QProcess::NormalExit)
        && (process.exitCode() == 0))
            retval = true;

    return retval;
}

void NodeDisk::loadFromFileSystem()
{
    if (fsLoaded) return;
    fsQDir.setPath(catPath);
    fsLoaded = true;
}

bool NodeDisk::isReachable()
{
    if (!fsLoaded) loadFromFileSystem();
    return fsQDir.exists();
}

void NodeDisk::osOpen() const
{
    if (!QDesktopServices::openUrl(QUrl(catPath)))
        Utils::errorMessageBox(NULL, "O/S open location failed");
}

bool NodeDisk::loadChildren()
{
    QSqlTableModel dirsModel;
    dirsModel.setTable("directories");
    dirsModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    dirsModel.setFilter(QString("parent = %1").arg(rootDirID));
    if (!dirsModel.select()) return false;
    while(dirsModel.canFetchMore()) dirsModel.fetchMore();

    qint64 numDirs = dirsModel.rowCount();
    for (qint64 i = 0; i < numDirs; i++)
    {
        NodeDir* newDir = new NodeDir(dirsModel.data(dirsModel.index(i, 0), Qt::DisplayRole).toLongLong(),
                                      dirsModel.data(dirsModel.index(i, 4), Qt::DisplayRole).toString(),
                                      dirsModel.data(dirsModel.index(i, 9), Qt::DisplayRole).toInt());
        addChild(newDir);
    }
    childrenLoaded = true;
    return true;
}

QString NodeDisk::summaryText() const
{
    QString text = "Disk: " + name + ". " + dirStats(rootDirID);
    return text;
}

void NodeDisk::removeFromDB()
{
    if (!dbman.startTransaction())
    {
        Utils::errorMessageBox(NULL, "Database Error:\ndelDisk: Transaction start error");
        emit deleteFinished(this, false);
        return;
    }

    QSqlQuery query;
    if (!query.exec(QString("delete from files where dirid in (select id from directories where diskid = %1)").arg(id)))
    {
        Utils::errorMessageBox(NULL, "Database Error:\ndelDisk: Query 1 fail");
        dbman.rollbackTransaction();
        emit deleteFinished(this, false);
        return;
    }

    if (!query.exec(QString("delete from directories where diskid = %1").arg(id)))
    {
        Utils::errorMessageBox(NULL, "Database Error:\ndelDisk: Query 2 fail");
        dbman.rollbackTransaction();
        emit deleteFinished(this, false);
        return;
    }

    if (!query.exec(QString("delete from disks where id = %1").arg(id)))
    {
        Utils::errorMessageBox(NULL, "Database Error:\ndelDisk: Query 4 fail");
        dbman.rollbackTransaction();
        emit deleteFinished(this, false);
        return;
    }

    if (!dbman.commitTransaction())
    {
        Utils::errorMessageBox(NULL, "Database Error:\ndelDisk: Commit fail");
        dbman.rollbackTransaction();
        emit deleteFinished(this, false);
        return;
    }

    emit deleteFinished(this, true);
}

bool NodeDisk::removeContentsFromDBNT() const
{
    // Deletes everything except the disks row
    // Doesn't work in a transaction
    // For use by Cataloguer in update mode, there will already be a transaction

    QSqlQuery query;
    if (!query.exec(QString("delete from files where dirid in (select id from directories where diskid = %1)").arg(id)))
    {
        Utils::errorMessageBox(NULL, "Database Error:\nUdelDisk: Query 1 fail");
        return false;
    }

    if (!query.exec(QString("delete from directories where diskid = %1").arg(id)))
    {
        Utils::errorMessageBox(NULL, "Database Error:\nUdelDisk: Query 2 fail");
        return false;
    }

    return true;
}

void NodeDisk::update(qint64 _catID, const QString& _name, const QString& _catPath,
                      qint64 _catTime, const QString& _deviceName, const QString& _fsLabel,
                      const QString& _fsType, qint64 _fsSize, qint64 _fsFree, int _isRoot, const QString& t_uuid)
{
    catID = _catID;
    name = _name;
    catPath = _catPath;
    catTime = _catTime;
    deviceName = _deviceName;
    fsLabel = _fsLabel;
    fsType = _fsType;
    fsSize = _fsSize;
    fsFree = _fsFree;
    isRoot = _isRoot;
    uuid = t_uuid;

    for(auto c : Node::children) delete c; // It seems there's a children in QObject as well which this class didn't originally derive from
    Node::children.clear();
    childrenLoaded = false;
}

NodeDisk* NodeDisk::createDisk(qint64 catID, const QString& name, const QString& catPath,
                               const QString& deviceName, const QString& fsLabel,
                               const QString& fsType, qint64 fsSize, qint64 fsFree, int isRoot, const QString& uuid)
{
    // Make a disk object and get the ID
    QSqlQuery query;
    if (!query.prepare("insert into disks ( catid,  name,  catpath,  cattime,  devname,  fslabel,  fstype,  fssize,  fsfree,  isroot,  uuid) "
                                  "values (:catid, :name, :catpath, :cattime, :devname, :fslabel, :fstype, :fssize, :fsfree, :isroot, :uuid)"))
    {
        qDebug() << "Create disk prep fail";
        return NULL;
    }

    qint64 catTime = QDateTime::currentDateTime().toSecsSinceEpoch();

    query.bindValue(":catid", catID);
    query.bindValue(":name", name);
    query.bindValue(":catpath", catPath);
    query.bindValue(":cattime", catTime);
    query.bindValue(":devname", deviceName);
    query.bindValue(":fslabel", fsLabel);
    query.bindValue(":fstype", fsType);
    query.bindValue(":fssize", fsSize);
    query.bindValue(":fsfree", fsFree);
    query.bindValue(":isroot", isRoot);
    query.bindValue(":uuid", uuid);

    if (!query.exec())
    {
        qDebug() << "Create disk query fail";
        return NULL;
    }

    qint64 newDiskID = query.lastInsertId().toLongLong();

    NodeDisk* newDisk = new NodeDisk(newDiskID, catID, name, catPath, catTime, deviceName, fsLabel,
                                     fsType, fsSize, fsFree, isRoot, QString(), QString(), uuid);
    return newDisk;
}
