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
#include <QSqlTableModel>

#include "globals.h"
#include "noderoot.h"
#include "nodecatalogue.h"
#include "nodedisk.h"
#include "utils.h"

#include "node.h"

NodeRoot* Node::rootNode = NULL;

Node::Node(qint64 t_type, qint64 t_id, const QString& t_name)
    : type(t_type), id(t_id), name(t_name)
{}

Node::~Node()
{
    for(auto item : children) delete item;
}

void Node::createRoot()
{
    rootNode = new NodeRoot();
    rootNode->loadChildren();
}

qint64 Node::numChildren() const
{
    return children.size();
}

Node* Node::getChild(qint64 index) const
{
    return children[index];
}

// Use the UI calling hasChildren() to trigger loading data
// This keeps the model one step in front of the GUI
bool Node::hasChildren()
{
    if (!childrenLoaded) loadChildren();
    return (children.size() > 0);
}

void Node::addChild(Node* newChild)
{
    Q_ASSERT(! ((type == TYPE_ROOT) && (newChild->getType() == TYPE_DIR)) );
    Q_ASSERT(! ((type == TYPE_CAT) && (newChild->getType() != TYPE_DISK)) );
    Q_ASSERT(! ((type == TYPE_DISK) && (newChild->getType() != TYPE_DIR)) );
    Q_ASSERT(! ((type == TYPE_DIR) && (newChild->getType() != TYPE_DIR)) );
    newChild->siblingIndex = numChildren();
    newChild->parent = this;
    children.append(newChild);
}

void Node::removeChild(Node* toDel)
{
    QVector<Node*>::iterator it = children.begin();
    QVector<Node*>::iterator after;
    while (*it != toDel) ++it;

    if (it == children.end())
    {
        qDebug() << "Error, TreeItem::deleteChild failed to find child in vector";
        return;
    }

    after = children.erase(it);

    for (; after != children.end(); after++)
    {
        --(*after)->siblingIndex;
    }
}

QString Node::dirStats(qint64 dirID)
{
    QString retval;

    QSqlQuery query;
    query.exec(QString("select count(*) from directories where parent = %1").arg(dirID));
    query.next();

    if (query.value(0).toLongLong() == 1)
        retval = query.value(0).toString() + " directory, ";
    else
        retval = query.value(0).toString() + " directories, ";


    query.exec(QString("select count(*) from files where dirid = %1").arg(dirID));
    query.next();

    if (query.value(0).toLongLong() == 1)
        retval += query.value(0).toString() + " file (";
    else
        retval += query.value(0).toString() + " files (";

    query.exec(QString("select sum(size) from files where dirid = %1").arg(dirID));
    query.next();
    retval += fileSizeToHR(query.value(0).toLongLong());
    retval += ")";

    return retval;
}

bool Node::rename(const QString&)
{
    Q_ASSERT(false);
    return false;
}

void Node::eachDiskInModel(QSqlTableModel& disksModel, std::function<void (NodeDisk*)> func)
{
    qint64 numDisks = disksModel.rowCount();
    for (qint64 i = 0; i < numDisks; i++)
    {
        NodeDisk* newDisk = new NodeDisk(
        /*id*/                  disksModel.data(disksModel.index(i, 0), Qt::DisplayRole).toLongLong(),
        /*catid*/               disksModel.data(disksModel.index(i, 1), Qt::DisplayRole).toLongLong(),
        /*name*/                disksModel.data(disksModel.index(i, 2), Qt::DisplayRole).toString(),
        /*catpath*/             disksModel.data(disksModel.index(i, 3), Qt::DisplayRole).toString(),
        /*cattime*/             disksModel.data(disksModel.index(i, 4), Qt::DisplayRole).toLongLong(),
        /*devname*/             disksModel.data(disksModel.index(i, 5), Qt::DisplayRole).toString(),
        /*fslabel*/             disksModel.data(disksModel.index(i, 6), Qt::DisplayRole).toString(),
        /*fstype*/              disksModel.data(disksModel.index(i, 7), Qt::DisplayRole).toString(),
        /*fssize*/              disksModel.data(disksModel.index(i, 8), Qt::DisplayRole).toLongLong(),
        /*fsfree*/              disksModel.data(disksModel.index(i, 9), Qt::DisplayRole).toLongLong(),
        /*isroot*/              disksModel.data(disksModel.index(i, 10), Qt::DisplayRole).toInt(),
        /*mountcmd*/            disksModel.data(disksModel.index(i, 11), Qt::DisplayRole).toString(),
        /*umountcmd*/           disksModel.data(disksModel.index(i, 12), Qt::DisplayRole).toString(),
        /*uuid*/                disksModel.data(disksModel.index(i, 13), Qt::DisplayRole).toString()
                            );

        if (!newDisk->loadRootDirID())
        {
            Utils::errorMessageBox(NULL, QString("Database Error:\neachDiskInModel: Failed to load root dir ID for disk %1").arg(newDisk->getID()));
            delete newDisk;
        }
        else
        {
            func(newDisk);
        }
    }
}
