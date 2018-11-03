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

#include "nodecatalogue.h"
#include "nodedisk.h"
#include "globals.h"
#include "noderoot.h"

#include "treemodel.h"

TreeModel::TreeModel(QObject* parent, NodeRoot* _rootNode)
    : QAbstractItemModel(parent), rootNode(_rootNode)
{
}

TreeModel::~TreeModel()
{
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return rootNode->numChildren();
    else
        return static_cast<Node*>(parent.internalPointer())->numChildren();
}

int TreeModel::columnCount(const QModelIndex&) const
{
    return 1; // Each node only ever has one column
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(row > -1);
    Q_ASSERT(column > -1);
    Node* target;
    Q_ASSERT(! (parent.isValid() && (parent.internalPointer() == NULL)));
    if (!parent.isValid()) target = rootNode;
    else target = static_cast<Node*>(parent.internalPointer());
    if (row >= target->numChildren()) return QModelIndex();
    QModelIndex qmi = createIndex(row, column, target->getChild(row));
    return qmi;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (index == QModelIndex()) return QModelIndex();
    Node* target = static_cast<Node*>(index.internalPointer());
    Q_ASSERT(target);
    Node* parent = target->getParent();
    if (parent == rootNode) return QModelIndex();
    return createIndex(parent->getSiblingIndex(), 0, parent);
}

bool TreeModel::hasChildren(const QModelIndex& parent) const
{
    Node* target = static_cast<Node*>(parent.internalPointer());
    if (!target) return rootNode->hasChildren();
    else return target->hasChildren();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    Node* target = static_cast<Node*>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        return target->getName();
    }
    else if (role == Qt::DecorationRole)
    {
        switch (target->getType())
        {
        case TYPE_CAT:
            return catalogueIcon;
        case TYPE_DISK:
            return diskIcon;
        case TYPE_DIR:
            return dirIcon;
        }
    }
    else if (role == ROLE_ID)
    {
        return target->getID();
    }
    else if (role == ROLE_TYPE)
    {
        return target->getType();
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Node* t = static_cast<Node*>(index.internalPointer());
    if ((t->getType() == TYPE_CAT) || (t->getType() == TYPE_DISK)) return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else return QAbstractItemModel::flags(index);
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        Node* target = static_cast<Node*>(index.internalPointer());
        if (target->rename(value.toString()))
        {
            emit dataChanged(index, index, {role});
            return true;
        }
    }
    return false;
}

void TreeModel::addCatalogue(NodeCatalogue* newCat)
{
    beginInsertRows(QModelIndex(), rootNode->numChildren(), rootNode->numChildren());
    rootNode->addChild(newCat);
    endInsertRows();
}

QModelIndex TreeModel::addDisk(NodeDisk* newDisk, std::function<void()> adder)
{
    if (newDisk->getCatID() == 0)
    {
        qint64 newRow = rootNode->numChildren();
        beginInsertRows(QModelIndex(), newRow, newRow);
        adder();
        endInsertRows();
        return index(newRow, 0);
    }
    else
    {
        QModelIndex catQmi = getQmiForCatID(newDisk->getCatID());
        Node* n = static_cast<Node*>(catQmi.internalPointer());
        qint64 newRow = n->numChildren();
        beginInsertRows(catQmi, newRow, newRow);
        adder();
        endInsertRows();
        return index(newRow, 0, catQmi);
    }
}

void TreeModel::removeNode(QModelIndex& qmiToDel, std::function<void()> remover)
{
    Node* toDel = static_cast<Node*>(qmiToDel.internalPointer());
    beginRemoveRows(qmiToDel.parent(), toDel->getSiblingIndex(), toDel->getSiblingIndex());
    remover();
    endRemoveRows();
}

QModelIndex TreeModel::getQmiForCatID(qint64 id)
{
    QModelIndexList matches = match(index(0, 0), ROLE_ID, id, -1, Qt::MatchExactly);
    foreach(QModelIndex qmi, matches)
    {
        Node* n = static_cast<Node*>(qmi.internalPointer());
        if (n->getType() == TYPE_CAT) return qmi;
    }
    return QModelIndex();
}

bool TreeModel::renameDisk(const QModelIndex& parentCatIndex, qint64 diskID, const QString& newName)
{
    QModelIndexList matches = match(index(0, 0, parentCatIndex), ROLE_ID, diskID, -1, Qt::MatchExactly);
    Q_ASSERT(matches.size() == 1);
    QModelIndex diskIndex = matches[0];
    Q_ASSERT(data(diskIndex, ROLE_TYPE) == TYPE_DISK);

    Node* node = static_cast<Node*>(diskIndex.internalPointer());
    NodeDisk* disk = static_cast<NodeDisk*>(node);
    if (disk->rename(newName))
    {
        emit dataChanged(diskIndex, diskIndex, {Qt::DisplayRole});
        return true;
    }
    return false;
}
