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

#ifndef CATALOGUER_H
#define CATALOGUER_H

#include <QObject>
#include <QSqlQuery>
#include <QStorageInfo>

class QDir;
class QString;
class NodeDisk;

#include "db.h"

class Cataloguer: public QObject
{
    Q_OBJECT

public:
    Cataloguer(qint64 catID, const QString& newDiskName, const QString& newPath);
    ~Cataloguer();

    int getError() const { return savedError; }
    const QStringList& getAccessDeniedPaths() const { return accessDeniedPaths; }

    void updateMode(NodeDisk* disk);
    void abort();

public slots:
    void go();

signals:
    void numObjectsFound(qint64 numObjects);
    void reindexing();
    void finished(NodeDisk* disk);

private:
    qint64 catID;
    QString newDiskName;
    QString newPath;

    void recurse(const QDir& dir, qint64 dirID); // throws int
    DB* cdb;
    NodeDisk* disk;
    qint64 totalDirs = 0;
    qint64 totalFiles = 0;
    QSqlQuery* dirQuery;
    QSqlQuery* fileQuery;
    QSqlQuery* numItemsQuery;
    QStorageInfo rootStorageInfo;
    QStorageInfo subCheckStorageInfo;
    bool abortNow = false;
    qint64 numObjects = 0;
    int savedError = 0;
    QStringList accessDeniedPaths;
};

#endif // CATALOGUER_H
