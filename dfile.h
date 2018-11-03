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

#ifndef DFILE_H
#define DFILE_H

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QString>

class DFile
{
public:
    DFile(qint64 id);
    bool loadFromDB();
    void loadFromFileSystem();

    qint64 getID() const { return id; }
    int getType() const { return type; }
    const QString& getFileName() const { return fileName; }
    const QString& getFOwner() const { return fOwner; }
    const QString& getFGroup() const { return fGroup; }
    const QString& getDiskName() const { return diskName; }
    const QString& getCatName() const { return catName; }
    const QString& getDiskPath() const { return diskPath; }

    const QString& getSizeFullText();
    const QString& getPermissionsText();
    const QString& getLastModifiedFullText();

    bool isReachable();
    bool isContainerReachable();
    void osOpen();
    void osOpenContainer();

private:
    bool dbLoaded = false;
    bool fsLoaded = false;

    qint64 id;

    // DB
    qint64 dirID;
    QString fileName;
    qint64 size;
    int type;
    qint64 modtime;
    QString fOwner;
    QString fGroup;
    qint64 qPermissions;
    QString catName;
    QString diskName;
    qint64 diskID;
    qint64 catID;

    // Calculated
    QDateTime qdtLastModified;
    QString sizeFullText;
    QString lastModifiedFullText;
    QString permissionsText;
    QString diskPath;
    QString fullURL;
    QString containerURL;
    QFile qfile;
    QFileInfo qfiContainer;
};

#endif // DFILE_H
