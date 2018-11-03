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
#include <QSqlTableModel>
#include <QDateTime>
#include <QFileDevice>
#include <QFont>

#include "globals.h"

#include "tablemodel.h"

const QString TableModel::dateFormat("dd/MM/yy hh:mm");

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableModel::~TableModel()
{
    clearData();
}

void TableModel::loadCat(qint64 catID)
{
    beginResetModel();

    clearData();

    mode = MODE_CAT;
    showingCatID = catID;

    cmodel = new QSqlTableModel();
    cmodel->setTable("disks");
    cmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    cmodel->setFilter(QString("catid = %1").arg(catID));
    cmodel->select();

    while(cmodel->canFetchMore()) cmodel->fetchMore();

    numDisks = cmodel->rowCount();
    endResetModel();
}

void TableModel::loadDir(qint64 dirID)
{
    beginResetModel();

    clearData();

    mode = MODE_DF;

    dmodel = new QSqlTableModel();
    dmodel->setTable("directories");
    dmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    dmodel->setFilter(QString("parent = %1").arg(dirID));
    dmodel->select();

    while(dmodel->canFetchMore()) dmodel->fetchMore();

    fmodel = new QSqlTableModel();
    fmodel->setTable("files");
    fmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    fmodel->setFilter(QString("dirid = %1").arg(dirID));
    fmodel->select();

    while(fmodel->canFetchMore()) fmodel->fetchMore();

    numDirs = dmodel->rowCount();
    numFiles = fmodel->rowCount();

    endResetModel();
}

void TableModel::clear()
{
    beginResetModel();
    clearData();
    mode = MODE_DF;
    endResetModel();
}

bool TableModel::reloadCat()
{
    if (mode == MODE_CAT)
    {
        loadCat(showingCatID);
        return true;
    }
    return false;
}

void TableModel::clearData()
{
    if (cmodel) delete cmodel;
    if (dmodel) delete dmodel;
    if (fmodel) delete fmodel;

    cmodel = dmodel = fmodel = NULL;
    numDisks = 0;
    numDirs = 0;
    numFiles = 0;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        if (section == 0) return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    if (role != Qt::DisplayRole) return QVariant();

    switch(section)
    {
        case 0:
            return "Name";
        case 1:
            return "Size";
        case 2:
            return "Modified";
        case 3:
            return "Owner";
        case 4:
            return "Group";
        case 5:
            return "Permissions";
    }

    return QVariant();
}

int TableModel::rowCount(const QModelIndex&) const
{
    if (mode == MODE_CAT)
        return numDisks;
    else
        return numDirs + numFiles;
}

int TableModel::columnCount(const QModelIndex&) const
{
    return NUM_COLUMNS;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int indexCol = index.column();

    if (mode == MODE_CAT)
    {
        if (role == ROLE_TYPE)
        {
            return TYPE_DISK; // The only object type displayed in MODE_CAT
        }
        else if (role == ROLE_ID)
        {
            QModelIndex newIndex = cmodel->index(index.row(), COL_DISKS_ID);
            return cmodel->data(newIndex, Qt::DisplayRole);
        }

        if (indexCol == 0)
        {
            QModelIndex newIndex = cmodel->index(index.row(), COL_DISKS_NAME);
            switch(role)
            {
            case Qt::DisplayRole:
            case ROLE_RAW: // return raw data for sorting
                return cmodel->data(newIndex, role);
            case Qt::DecorationRole:
                return diskIcon;
            }
        }
    }
    else // mode dirs+files
    {
        if (index.row() < numDirs) // Asking for a directory
        {
            switch(role)
            {
                case ROLE_TYPE:
                    return TYPE_DIR;

                case ROLE_ID:
                {
                    QModelIndex newIndex = dmodel->index(index.row(), COL_DIRS_ID);
                    return dmodel->data(newIndex, Qt::DisplayRole);
                }


                case Qt::DisplayRole:
                {
                    QModelIndex newIndex = dmodel->index(index.row(), dirColumnConvert[indexCol]);
                    switch(indexCol)
                    {
                        case 1:
                        {
                            qint64 numItems = dmodel->data(newIndex, role).toLongLong();
                            if (numItems == 1) return QString("%1 item").arg(numItems);
                            else return QString("%1 items").arg(numItems);
                        }
                        case 2:
                        {
                            QVariant qv = dmodel->data(newIndex, role);
                            QDateTime qdt;
                            qdt.setSecsSinceEpoch(qv.toLongLong());
                            return qdt.toString(dateFormat);
                        }
                        case 5:
                        {
                            return qPermissionsToText(dmodel->data(newIndex, role).toLongLong());
                        }
                        case 0:
                        case 3:
                        case 4:
                            return dmodel->data(newIndex, role);
                    }
                    [[fallthrough]]; // It won't. Placate the compiler.
                }
                case Qt::DecorationRole:
                    if (indexCol == 0) return dirIcon;
                    else break;

                case ROLE_RAW: // return raw data for sorting
                {
                    QModelIndex newIndex = dmodel->index(index.row(), dirColumnConvert[indexCol]);
                    return dmodel->data(newIndex, Qt::DisplayRole);
                }

                case Qt::TextAlignmentRole:
                    if (indexCol == 1) return QVariant(Qt::AlignRight | Qt::AlignVCenter);
                    else break;
            }
        }
        else // Asking for a file
        {
            int type = fmodel->data(fmodel->index(index.row() - numDirs, COL_FILES_TYPE), Qt::DisplayRole).toInt();

            switch(role)
            {
            case ROLE_TYPE:
            {
                return type;
            }

            case ROLE_ID:
            {
                QModelIndex newIndex = fmodel->index(index.row() - numDirs, COL_FILES_ID);
                return fmodel->data(newIndex, Qt::DisplayRole);
            }

            case Qt::DisplayRole:
            {
                QModelIndex newIndex = fmodel->index(index.row() - numDirs, fileColumnConvert[indexCol]);
                QVariant qv = fmodel->data(newIndex, role);
                switch(indexCol)
                {
                    case 1:
                        if (type > TYPE_FILE) return QVariant();
                        return fileSizeToHR(qv.toLongLong());
                    case 2:
                    {
                        QDateTime qdt;
                        qdt.setSecsSinceEpoch(qv.toLongLong());
                        return qdt.toString(dateFormat);
                    }
                    case 5:
                    {
                        return qPermissionsToText(qv.toLongLong());
                    }
                    case 0:
                    case 3:
                    case 4:
                        return qv;
                }
            }
            [[fallthrough]]; // It won't. Placate the compiler.

            case Qt::DecorationRole:

                if (indexCol == 0)
                {
                    if (type == TYPE_FILE) return fileIcon;
                    if (type == TYPE_SYMLINK) return fileLinkIcon;
                    if (type == TYPE_OTHERFILEUNKNOWN) return fileCogIcon;
                    else return QVariant();
                }
                else break;

            case Qt::TextAlignmentRole:
                if (indexCol == 1) return QVariant(Qt::AlignRight | Qt::AlignVCenter);
                else break;

            case Qt::FontRole:
                if (indexCol == 0)
                {
                    if (type == TYPE_SYMLINK)
                    {
                        QFont font;
                        font.setItalic(true);
                        return font;
                    }
                }
                break;

            case ROLE_RAW: // return raw data for sorting
                QModelIndex newIndex = fmodel->index(index.row() - numDirs, fileColumnConvert[indexCol]);
                return fmodel->data(newIndex, Qt::DisplayRole);
            }
        }
    }

    return QVariant();
}


Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || (mode != MODE_CAT)) return QAbstractItemModel::flags(index);
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        qint64 type = data(index, ROLE_TYPE).toInt();
        if (type == TYPE_DISK)
        {
            qint64 id = data(index, ROLE_ID).toLongLong();
            emit requestRenameDisk(id, value.toString());
        }
    }
    return false;
}
