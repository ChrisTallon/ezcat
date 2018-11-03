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

#ifndef NODE_H
#define NODE_H

#include <functional>
#include <QVector>
#include <QString>
#include <QSqlTableModel>

class NodeRoot;
class NodeCatalogue;
class NodeDisk;

class Node
{
public:
    virtual ~Node();

    qint64 getID() const { return id; }
    qint64 getType() const { return type; }
    const QString getName() const { return name; }
    Node* getParent() const { return parent; }
    qint64 getSiblingIndex() const { return siblingIndex; }

    qint64 numChildren() const;
    Node* getChild(qint64 index) const;
    bool hasChildren();
    void addChild(Node* newChild);
    void removeChild(Node* toDel);

    virtual bool rename(const QString& value);
    virtual bool loadChildren() =0;
    virtual QString summaryText() const =0;

    static void createRoot();
    static NodeRoot* getRootNode() { return rootNode; }
    static QString dirStats(qint64 dirID);
    static void eachDiskInModel(QSqlTableModel& disksModel, std::function<void (NodeDisk *)> func);

protected:
    Node(qint64 type, qint64 id, const QString& name);
    static NodeRoot* rootNode;
    Node* parent;
    bool childrenLoaded = false;
    qint64 siblingIndex;
    QVector<Node*> children;

    qint64 type;
    qint64 id;
    QString name;
};

#endif // NODE_H
