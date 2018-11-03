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
#include "searchresult.h"

#include "searchmodel.h"

SearchModel::SearchModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

SearchModel::~SearchModel()
{
    foreach(SearchResult* sr, results)
    {
        delete sr;
    }
}

QVariant SearchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();
    if (role != Qt::DisplayRole) return QVariant();

    switch(section)
    {
        case 0:
            return "Name";
        case 1:
            return "Location";
    }

    return QVariant();
}

int SearchModel::rowCount(const QModelIndex& /*parent*/) const
{
    return results.size();
}

int SearchModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

void SearchModel::search(QString& text)
{
    /* SQL Injection.
     * QSqlTableModel is the right class to be using here, but there is no prepare/bind or
     * argument validation / escaping functionality.
     * QSqlQueryModel could also be used with its supposedly public setQuery(QSqlQuery&)
     * function, but actually this is protected in the Qt lib.
     * Could subclass QSqlQueryModel to get around that protected...
     * But for now, since QSqlTableModel uses a QSqlQuery to talk to the DB, *and*
     * QSqlQuery for SQLite3 is not a multi statement API, it is possibly safe to
     * leave these 4 QSqlTableModel uses as they are.
     */

    QSqlTableModel* model = new QSqlTableModel();
    model->setTable("catalogues");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setFilter(QString("UPPER(name) like '%%1%'").arg(text.toUpper())); // FIXME DB
    model->select();

    while(model->canFetchMore()) model->fetchMore();

    numCats = model->rowCount();

    for (qint64 i = 0; i < numCats; i++)
    {
        SearchResult* sr = new SearchResult();
        sr->setType(TYPE_CAT);
        sr->setID(model->data(model->index(i, 0)).toLongLong());
        QString qsName = model->data(model->index(i, 1)).toString();
        sr->setName(qsName);
        sr->calcLocation();
        results.append(sr);
    }

    delete model;


    model = new QSqlTableModel();
    model->setTable("disks");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setFilter(QString("UPPER(name) like '%%1%'").arg(text.toUpper())); // FIXME DB
    model->select();

    while(model->canFetchMore()) model->fetchMore();

    numDisks = model->rowCount();

    for (qint64 i = 0; i < numDisks; i++)
    {
        SearchResult* sr = new SearchResult();
        sr->setType(TYPE_DISK);
        sr->setID(model->data(model->index(i, 0)).toLongLong());
        QString qsName = model->data(model->index(i, 2)).toString();
        sr->setName(qsName);
        sr->setCatID(model->data(model->index(i, 1)).toLongLong());
        sr->calcLocation();
        results.append(sr);
    }

    delete model;

    model = new QSqlTableModel();
    model->setTable("directories");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setFilter(QString("UPPER(name) like '%%1%'").arg(text.toUpper())); // FIXME DB
    model->select();

    while(model->canFetchMore()) model->fetchMore();

    numDirs = model->rowCount();

    for (qint64 i = 0; i < numDirs; i++)
    {
        SearchResult* sr = new SearchResult();
        sr->setType(TYPE_DIR);
        sr->setID(model->data(model->index(i, 0)).toLongLong());
        QString qsName = model->data(model->index(i, 4)).toString();
        sr->setName(qsName);
        sr->setParentDirID(model->data(model->index(i, 2)).toLongLong());
        sr->calcLocation();
        results.append(sr);
    }

    delete model;

    model = new QSqlTableModel();
    model->setTable("files");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setFilter(QString("UPPER(name) like '%%1%'").arg(text.toUpper())); // FIXME DB
    model->select();

    while(model->canFetchMore()) model->fetchMore();

    numFiles = model->rowCount();

    for (qint64 i = 0; i < numFiles; i++)
    {
        SearchResult* sr = new SearchResult();
        sr->setType(model->data(model->index(i, 4)).toInt());
        sr->setID(model->data(model->index(i, 0)).toLongLong());
        QString qsName = model->data(model->index(i, 2)).toString();
        sr->setName(qsName);
        sr->setParentDirID(model->data(model->index(i, 1)).toLongLong());
        sr->calcLocation();
        results.append(sr);
    }

    delete model;
}

QVariant SearchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    int indexCol = index.column();
    SearchResult* sr = results[index.row()];

    switch(role)
    {
    case Qt::DisplayRole:
        if (indexCol == 0) return sr->getName();
        if (indexCol == 1) return sr->getLocation();
        break;
    case Qt::DecorationRole:
        if (indexCol == 0)
        {
            int testType = sr->getType();
            if (testType == TYPE_CAT) return catalogueIcon;
            if (testType == TYPE_DISK) return diskIcon;
            if (testType == TYPE_DIR) return dirIcon;
            if (testType == TYPE_FILE) return fileIcon;
            if (testType == TYPE_SYMLINK) return fileLinkIcon;
            if (testType == TYPE_OTHERFILEUNKNOWN) return fileCogIcon;
        }
        else if (indexCol == 1)
        {
            if (sr->getLocationType() == TYPE_CAT) return catalogueIcon;
            if (sr->getLocationType() == TYPE_DISK) return diskIcon;
        }
        break;
    }

    return QVariant();
}

QModelIndex SearchModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
    if (row >= results.size()) return QModelIndex();
    return createIndex(row, column, results[row]);
}
