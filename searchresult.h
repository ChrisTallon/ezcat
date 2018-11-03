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

#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QList>
#include <QString>

class DFile;
class DDir;
class NodeDisk;

class SearchResult
{

public:
    SearchResult();
    ~SearchResult();

    qint64 getType() const { return type; }
    qint64 getID() const { return id; }
    const QString& getName() const { return name; }
    const QString& getLocation() const { return location; }
    const QList<QPair<qint64,qint64>>& getFullIDLocation() const { return fullIDLocation; }
    qint64 getLocationType() const { return locationType; }

    void setType(qint64);
    void setID(qint64);
    void setName(QString&);
    void setCatID(qint64); // if a disk
    void setParentDirID(qint64); // if a dir or file
    void calcLocation();

    void loadDObject();
    bool isReachable();
    bool isContainerReachable();
    void osOpen();
    void osOpenContainer();
    DFile* getDFile();
    DDir* getDDir();
    NodeDisk* getNodeDisk();

private:
    qint64 type;
    qint64 id;
    qint64 catid;
    qint64 parentDirID;
    QString name;
    QString location;
    QList<QPair<qint64,qint64>> fullIDLocation;
    qint64 locationType = TYPE_INVALID;
    bool dObjectLoaded = false;
    void* dObject = NULL;

    bool calcLocation2(qint64& nodeID, qint64& nodeType, QList<QString*>& locationParts);
};

#endif // SEARCHRESULT_H
