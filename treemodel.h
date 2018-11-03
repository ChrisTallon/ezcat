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

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <functional>
#include <QAbstractItemModel>

class Node;
class NodeRoot;
class NodeCatalogue;
class NodeDisk;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject *parent, NodeRoot* rootNode);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void addCatalogue(NodeCatalogue* newCat);
    QModelIndex addDisk(NodeDisk* newDisk, std::function<void()> adder);
    void removeNode(QModelIndex &qmiToDel, std::function<void()> remover);
    QModelIndex getQmiForCatID(qint64 id);

    bool renameDisk(const QModelIndex &parentCatIndex, qint64 diskID, const QString &newName);

private:
    NodeRoot* rootNode;
};

#endif // TREEMODEL_H
