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

#include <QSqlTableModel>

#include "globals.h"
#include "nodecatalogue.h"
#include "nodedisk.h"
#include "utils.h"

#include "noderoot.h"

NodeRoot::NodeRoot()
    : Node(TYPE_ROOT, -1, QString())
{
    parent = NULL;
}

bool NodeRoot::loadChildren()
{
    QSqlTableModel catsModel;
    catsModel.setTable("catalogues");
    catsModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!catsModel.select())
    {
        Utils::errorMessageBox("Database Error:\nNodeRoot: Query 1 fail");
        return false;
    }

    while(catsModel.canFetchMore()) catsModel.fetchMore();

    numCats = catsModel.rowCount();

    for (qint64 i = 0; i < numCats; i++)
    {
        NodeCatalogue* newCat = new NodeCatalogue(catsModel.data(catsModel.index(i, 0), Qt::DisplayRole).toLongLong(),
                                          catsModel.data(catsModel.index(i, 1), Qt::DisplayRole).toString());
        addChild(newCat);
    }


    QSqlTableModel disksModel;
    disksModel.setTable("disks");
    disksModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    disksModel.setFilter(QString("catid = 0"));

    if (!disksModel.select())
    {
        Utils::errorMessageBox("Database Error:\nNodeRoot: Query 2 fail");
        return false;
    }

    while(disksModel.canFetchMore()) disksModel.fetchMore();
    numDisks = disksModel.rowCount();

    Node::eachDiskInModel(disksModel, [&] (NodeDisk* newDisk)
    {
        addChild(newDisk);
    });

    childrenLoaded = true;
    return true;
}

QString NodeRoot::summaryText() const
{
    return QString();
}

void NodeRoot::eachCat(std::function<void (NodeCatalogue *)> func)
{
    for (qint64 i = 0; i < children.size(); i++)
    {
        if ((children[i])->getType() == TYPE_CAT) func(static_cast<NodeCatalogue*>(children[i]));
    }
}

NodeCatalogue *NodeRoot::catFromID(qint64 catID) const
{
    for (qint64 i = 0; i < children.size(); i++)
    {
        if ((children[i]->getType() == TYPE_CAT) && (children[i]->getID() == catID)) return static_cast<NodeCatalogue*>(children[i]);
    }
    return NULL;
}

NodeDisk *NodeRoot::diskFromID(qint64 diskID) const
{
    for (qint64 i = 0; i < children.size(); i++)
    {
        if ((children[i]->getType() == TYPE_DISK) && (children[i]->getID() == diskID)) return static_cast<NodeDisk*>(children[i]);
    }
    return NULL;
}

NodeDisk *NodeRoot::diskFromIDRecursive(qint64 diskID) const
{
    NodeDisk* n = diskFromID(diskID);
    if (!n)
    {
        for (qint64 i = 0; i < children.size(); i++)
        {
            if ((children[i])->getType() == TYPE_CAT)
            {
                n = static_cast<NodeCatalogue*>(children[i])->diskFromID(diskID);
                if (n) break;
            }
        }
    }
    return n;
}
