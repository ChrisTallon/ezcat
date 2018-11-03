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

#ifndef DDIR_H
#define DDIR_H

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QString>

class DDir
{
public:
    DDir(qint64 id);
    bool loadFromDB();
    void loadFromFileSystem();

    qint64 getID() const { return id; }
    const QString& getDirName() const { return dirName; }
    const QString& getFOwner() const { return fOwner; }
    const QString& getFGroup() const { return fGroup; }
    const QString& getDiskName() const { return diskName; }
    const QString& getCatName() const { return catName; }
    const QString& getDiskPath() const { return diskPath; }
    bool isAccessDenied() const { return accessDenied; }

    const QString& getLastModifiedFullText();
    const QString& getPermissionsText();
    const QString& getDirStats();
    const QString& getSubContents();

    const QString& getSizeFullText();
    bool isReachable();
    void osOpen();

private:
    bool dbLoaded = false;
    bool fsLoaded = false;

    qint64 id;

    // DB
    qint64 diskID;
    qint64 parentDirID;
    qint64 numItems;
    QString dirName;
    qint64 modtime;
    QString fOwner;
    QString fGroup;
    qint64 qPermissions;
    QString catName;
    QString diskName;
    qint64 catID;
    bool accessDenied = false;

    // Calculated
    QDateTime qdtLastModified;
    QString lastModifiedFullText;
    QString permissionsText;
    QString diskPath;
    QString fullURL;
    QString containerURL;
    QDir qdir;
    QFileInfo qfiContainer;
    QString dirStats;
    QString dirSubContents;
    void recurse(qint64 dirID, qint64 &numDirectories, qint64 &numFiles, qint64& totalSize) const;
};


#endif // DDIR_H
