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

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>

class QSqlTableModel;

#define NUM_COLUMNS 6

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableModel(QObject *parent);
    ~TableModel();

    int getMode() const { return mode; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void loadCat(qint64 catID);
    void loadDir(qint64 dirID);
    bool reloadCat();
    void clear();

    const static int MODE_CAT = 1; // Displaying disks within a catalogue
    const static int MODE_DF = 2; // Displaying directories and files

signals:
    void requestRenameDisk(qint64 diskID, QString newName);

private:
    QSqlTableModel* cmodel = NULL;
    QSqlTableModel* fmodel = NULL;
    QSqlTableModel* dmodel = NULL;

    /* This model exposes columns:
     * 0 Name
     * 1 Size
     * 2 Modified
     * 3 Owner
     * 4 Group
     * 5 Permissions
    */

    const static int COL_DISKS_ID = 0;
    const static int COL_DISKS_NAME = 2;

    const static int COL_DIRS_ID = 0;
    const static int COL_DIRS_NUMITEMS = 3;
    const static int COL_DIRS_NAME = 4;
    const static int COL_DIRS_MODTIME = 5;
    const static int COL_DIRS_FOWNER = 6;
    const static int COL_DIRS_FGROUP = 7;
    const static int COL_DIRS_QPERMS = 8;

    const static int COL_FILES_ID = 0;
    const static int COL_FILES_NAME = 2;
    const static int COL_FILES_SIZE = 3;
    const static int COL_FILES_TYPE = 4;
    const static int COL_FILES_MODTIME = 5;
    const static int COL_FILES_FOWNER = 6;
    const static int COL_FILES_FGROUP = 7;
    const static int COL_FILES_QPERMS = 8;

    const int dirColumnConvert[NUM_COLUMNS] =  { COL_DIRS_NAME, COL_DIRS_NUMITEMS, COL_DIRS_MODTIME, COL_DIRS_FOWNER, COL_DIRS_FGROUP, COL_DIRS_QPERMS };
    const int fileColumnConvert[NUM_COLUMNS] = { COL_FILES_NAME, COL_FILES_SIZE, COL_FILES_MODTIME, COL_FILES_FOWNER, COL_DIRS_FGROUP, COL_FILES_QPERMS };

    qint64 numDisks = 0;
    qint64 numDirs = 0;
    qint64 numFiles = 0;

    int mode = MODE_CAT;
    qint64 showingCatID = -1;

    const static QString dateFormat;

    void clearData();

};

#endif // TABLEMODEL_H
