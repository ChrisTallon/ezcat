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
#include "utils.h"

#include "dfile.h"

DFile::DFile(qint64 t_id)
    : id(t_id)
{    
}

bool DFile::loadFromDB()
{
    if (dbLoaded) return true;

    QSqlQuery query;
    if (!query.exec(QString("select dirid,name,size,type,modtime,fowner,fgroup,qpermissions from files where id = %1").arg(id))) return false;
    if (!query.next()) return false;

    dirID = query.value(0).toLongLong();
    fileName = query.value(1).toString();
    size = query.value(2).toLongLong();
    type = query.value(3).toInt();
    modtime = query.value(4).toLongLong();
    fOwner = query.value(5).toString();
    fGroup = query.value(6).toString();
    qPermissions = query.value(7).toLongLong();

    qdtLastModified.setSecsSinceEpoch(modtime);

    QList<QString> parents;
    qint64 parentDirID = dirID;
    while(true)
    {
        if (!query.exec(QString("select diskid,parent,name from directories where id = %1").arg(parentDirID))) return false;
        if (!query.next()) return false;
        parentDirID = query.value(1).toLongLong();
        diskID = query.value(0).toLongLong();
        if (parentDirID == 0) break;
        parents.prepend(query.value(2).toString());
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

    containerPath = query.value(2).toString();
    containerPath += diskPath;
    fullPath = containerPath + "/" + fileName;

    if (catID)
    {
        if (!query.exec(QString("select name from catalogues where id = %1").arg(catID))) return false;
        if (!query.next()) return false;
        catName = query.value(0).toString();
    }
    dbLoaded = true;
    return true;
}

void DFile::loadFromFileSystem()
{
    if (fsLoaded) return;

    qfile.setFileName(fullPath);
    qfiContainer = QFileInfo(containerPath);

    fsLoaded = true;
}

const QString &DFile::getSizeFullText()

{
    if (sizeFullText.isEmpty())
    {
        sizeFullText = fileSizeToHR(size);
        sizeFullText.append(" (");
        sizeFullText.append(QLocale(QLocale::English).toString(size));
        sizeFullText.append(" B)");
    }
    return sizeFullText;
}

const QString &DFile::getPermissionsText()
{
    if (permissionsText.isEmpty())
    {
        permissionsText = qPermissionsToText(qPermissions);
    }
    return permissionsText;
}

bool DFile::isReachable()
{
    if (!fsLoaded) loadFromFileSystem();
    return qfile.exists();
}

bool DFile::isContainerReachable()
{
    if (!fsLoaded) loadFromFileSystem();
    return (qfiContainer.exists() && qfiContainer.isDir());
}

void DFile::osOpen()
{
    if (!isReachable()) return;
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath)))
        Utils::errorMessageBox("O/S open file failed");
}

void DFile::osOpenContainer()
{
    if (!isContainerReachable()) return;
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(containerPath)))
        Utils::errorMessageBox("O/S open file's location failed");
}

const QString& DFile::getLastModifiedFullText()
{
    if (lastModifiedFullText.isEmpty())
    {
        lastModifiedFullText = qdtLastModified.toString("dddd, d MMMM yyyy hh:mm:ss t");
    }
    return lastModifiedFullText;
}
