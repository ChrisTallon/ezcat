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
#include <QSqlQuery>

#include "globals.h"
#include "ddir.h"
#include "dfile.h"
#include "nodecatalogue.h"
#include "nodedisk.h"
#include "noderoot.h"
#include "utils.h"

#include "searchresult.h"

SearchResult::SearchResult()
{
}

SearchResult::~SearchResult()
{
    if (dObject)
    {
        if (type == TYPE_DIR)
        {
            DDir* ddir = static_cast<DDir*>(dObject);
            delete ddir;
        }
        else if (type >= TYPE_FILE)
        {
            DFile* dfile = static_cast<DFile*>(dObject);
            delete dfile;
        }
    }
}

void SearchResult::setType(qint64 _type)
{
    type = _type;
}

void SearchResult::setID(qint64 _id)
{
    id = _id;
}

void SearchResult::setName(QString& _name)
{
    name = _name;
}

// If a disk
void SearchResult::setCatID(qint64 _catid)
{
    Q_ASSERT(type == TYPE_DISK);
    catid = _catid;
}

void SearchResult::setParentDirID(qint64 _parentDirID)
{
    parentDirID = _parentDirID;
}

void SearchResult::calcLocation()
{
    qint64 currentID = id;
    qint64 currentType = type;
    QList<QString*> locationParts;
    while(calcLocation2(currentID, currentType, locationParts));

    qint64 sSize = 0;
    foreach(QString* qsp, locationParts) sSize += qsp->size();
    location.reserve(sSize + 1);
    for(auto i = locationParts.end() - 1; i >= locationParts.begin(); i--) location += **i;
    qDeleteAll(locationParts);
}

bool SearchResult::calcLocation2(qint64& nodeID, qint64& nodeType, QList<QString*>& locationParts)
{
    fullIDLocation.prepend(QPair<qint64,qint64>(nodeType, nodeID));

    if (nodeType == TYPE_CAT)
    {
        QSqlQuery query;
        if (!query.exec(QString("select name from catalogues where id = %1").arg(nodeID))) return false;
        query.next();
        if ((nodeID != id) || (nodeType != type))
        {
            locationParts.append(new QString("/"));
            locationParts.append(new QString(query.value(0).toString()));
            locationType = TYPE_CAT;
        }
        return false;
    }
    else if (nodeType == TYPE_DISK)
    {
        QSqlQuery query;
        if (!query.exec(QString("select catid, name from disks where id = %1").arg(nodeID))) return false;
        query.next();
        if ((nodeID != id) || (nodeType != type))
        {
            locationParts.append(new QString(":"));
            locationParts.append(new QString(query.value(1).toString()));
            locationType = TYPE_DISK;
        }

        nodeID = query.value(0).toLongLong();
        if (nodeID == 0)
        {
            fullIDLocation.prepend(QPair<int,int>(TYPE_CAT, 0));
            return false;
        }
        nodeType = TYPE_CAT;
    }
    else if (nodeType == TYPE_DIR)
    {
        QSqlQuery query;
        if (!query.exec(QString("select diskid, parent, name from directories where id = %1").arg(nodeID))) return false;
        query.next();
        qint64 possibleParent = query.value(1).toLongLong();
        if (possibleParent > 0)
        {
            if ((nodeID != id) || (nodeType != type))
            {
                locationParts.append(new QString(query.value(2).toString()));
                locationParts.append(new QString("/"));
            }

            nodeID = possibleParent;
            nodeType = TYPE_DIR;
        }
        else
        {
            nodeID = query.value(0).toLongLong();
            nodeType = TYPE_DISK;
        }
    }
    else if (nodeType >= TYPE_FILE)
    {
        QSqlQuery query;
        if (!query.exec(QString("select dirid from files where id = %1").arg(nodeID))) return false;
        query.next();
        nodeID = query.value(0).toLongLong();
        nodeType = TYPE_DIR;
    }

    return true;
}

void SearchResult::loadDObject()
{
    if (dObjectLoaded) return;
    dObjectLoaded = true;

    if (type == TYPE_DISK)
    {
        Q_ASSERT(fullIDLocation.size() == 2);
        if (fullIDLocation[0].second == 0) // Disk is in root
        {
            NodeDisk* foundDisk = Node::getRootNode()->diskFromID(fullIDLocation[1].second);
            dObject = foundDisk;
        }
        else
        {
            NodeCatalogue* foundCatalogue = Node::getRootNode()->catFromID(fullIDLocation[0].second);
            Q_ASSERT(foundCatalogue);
            NodeDisk* foundDisk = foundCatalogue->diskFromID(fullIDLocation[1].second);
            dObject = foundDisk;
        }

    }
    else if (type == TYPE_DIR)
    {
        DDir* ddir = new DDir(id);
        if (!ddir->loadFromDB()) Utils::errorMessageBox(NULL, "Database error");
        ddir->loadFromFileSystem();
        dObject = ddir;
    }
    else if (type >= TYPE_FILE)
    {
        DFile* dfile = new DFile(id);
        if (!dfile->loadFromDB()) Utils::errorMessageBox(NULL, "Database error");
        dfile->loadFromFileSystem();
        dObject = dfile;
    }
}

bool SearchResult::isReachable()
{
    if (type == TYPE_DISK)
    {
        return static_cast<NodeDisk*>(dObject)->isReachable();
    }
    else if (type == TYPE_DIR)
    {
        return static_cast<DDir*>(dObject)->isReachable();
    }
    else if (type >= TYPE_FILE)
    {
        return static_cast<DFile*>(dObject)->isReachable();
    }
    return false;
}

bool SearchResult::isContainerReachable()
{
    if (type >= TYPE_FILE)
    {
        return static_cast<DFile*>(dObject)->isContainerReachable();
    }
    return false;
}

void SearchResult::osOpen()
{
    Q_ASSERT(dObjectLoaded);

    if (type == TYPE_DISK)
    {
        return static_cast<NodeDisk*>(dObject)->osOpen();
    }
    if (type == TYPE_DIR)
    {
        return static_cast<DDir*>(dObject)->osOpen();
    }
    else if (type == TYPE_FILE) // block types > TYPE_FILE
    {
        return static_cast<DFile*>(dObject)->osOpen();
    }
}

void SearchResult::osOpenContainer()
{
    if (type >= TYPE_FILE)
        return static_cast<DFile*>(dObject)->osOpenContainer();
}

DFile *SearchResult::getDFile()
{
    if (type >= TYPE_FILE) return static_cast<DFile*>(dObject);
    return NULL;
}

DDir *SearchResult::getDDir()
{
    if (type == TYPE_DIR) return static_cast<DDir*>(dObject);
    return NULL;
}

NodeDisk *SearchResult::getNodeDisk()
{
    if (type == TYPE_DISK) return static_cast<NodeDisk*>(dObject);
    return NULL;
}
