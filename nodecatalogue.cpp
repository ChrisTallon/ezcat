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
#include "db.h"
#include "utils.h"
#include "nodedisk.h"
#include "utils.h"

#include "nodecatalogue.h"

NodeCatalogue::NodeCatalogue(qint64 t_id, const QString& t_name)
    : Node(TYPE_CAT, t_id, t_name)
{}

bool NodeCatalogue::rename(const QString& _newName)
{
    QString newName = _newName.trimmed();
    if (newName.isEmpty()) return false;

    if (!db.startTransaction()) { Utils::errorMessageBox("Database Error:\nrename: Transaction start error"); return false; }

    QSqlQuery query;
    query.prepare("update catalogues set name = :newname where id = :id");
    query.bindValue(":newname", newName);
    query.bindValue(":id", id);
    if (query.exec())
    {
        if (db.commitTransaction())
        {
            name = newName;
            return true;
        }
        return false;
    }
    else
    {
        db.rollbackTransaction();
        return false;
    }
}

bool NodeCatalogue::loadChildren()
{
    QSqlTableModel disksModel;
    disksModel.setTable("disks");
    disksModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    disksModel.setFilter(QString("catid = %1").arg(id));

    if (!disksModel.select())
    {
        Utils::errorMessageBox("RootNode: Failed to execute disks query");
        return false;
    }

    while(disksModel.canFetchMore()) disksModel.fetchMore();

    Node::eachDiskInModel(disksModel, [&] (NodeDisk* newDisk)
    {
        addChild(newDisk);
    });

    childrenLoaded = true;
    return true;
}


QString NodeCatalogue::summaryText() const
{
    QString text = "Catalogue: " + name + ". " + QString::number(numChildren()) + " disks.";
    return text;
}

void NodeCatalogue::removeFromDB()
{
    if (!db.startTransaction())
    {
        Utils::errorMessageBox("Database Error:\nremoveFromDB: Start transaction error");
        deleteFinished(this, false);
        return;
    }

    QSqlQuery query;
    if (!query.exec(QString("delete from files where dirid in (select id from directories where diskid in (select id from disks where catid = %1))").arg(id)))
    {
        Utils::errorMessageBox("Database Error:\nremoveFromDB: Query 1 fail");
        db.rollbackTransaction();
        deleteFinished(this, false);
        return;
    }

    if (!query.exec(QString("delete from directories where diskid in (select id from disks where catid = %1)").arg(id)))
    {
        Utils::errorMessageBox("Database Error:\nremoveFromDB: Query 2 fail");
        db.rollbackTransaction();
        deleteFinished(this, false);
        return;
    }

    if (!query.exec(QString("delete from disks where catid = %1").arg(id)))
    {
        Utils::errorMessageBox("Database Error:\nremoveFromDB: Query 3 fail");
        db.rollbackTransaction();
        deleteFinished(this, false);
        return;
    }

    if (!query.exec(QString("delete from catalogues where id = %1").arg(id)))
    {
        Utils::errorMessageBox("Database Error:\nremoveFromDB: Query 4 fail");
        db.rollbackTransaction();
        deleteFinished(this, false);
        return;
    }

    if (!db.commitTransaction())
    {
        Utils::errorMessageBox("Database Error:\nremoveFromDB: Commit transaction fail");
        db.rollbackTransaction();
        deleteFinished(this, false);
        return;
    }

    deleteFinished(this, true);
}

NodeCatalogue* NodeCatalogue::createCatalogue(const QString& name)
{
    QSqlQuery query;
    query.prepare("insert into catalogues (name) values (:newname)");
    query.bindValue(":newname", name);
    if (!query.exec())
    {
        Utils::errorMessageBox("Database Error:\ncreateCatalogue: Query fail");
        return NULL;
    }
    qint64 newCatID = query.lastInsertId().toLongLong();
    NodeCatalogue* newCat = new NodeCatalogue(newCatID, name);
    return newCat;
}

NodeDisk* NodeCatalogue::diskFromID(qint64 diskID) const
{
    for (qint64 i = 0; i < Node::children.size(); i++)
    {
        if (Node::children[i]->getID() == diskID) return static_cast<NodeDisk*>(Node::children[i]);
    }
    return NULL;
}
