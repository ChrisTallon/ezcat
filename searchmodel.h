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

#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractTableModel>
#include <QList>

class QSqlTableModel;
class SearchResult;

class SearchModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SearchModel(QObject *parent = nullptr);
    ~SearchModel();

    qint64 getNumCats() const { return numCats; }
    qint64 getNumDisks() const { return numDisks; }
    qint64 getNumDirs() const { return numDirs; }
    qint64 getNumFiles() const { return numFiles; }

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void search(QString &text);

private:
    QList<SearchResult*> results;
    qint64 numCats;
    qint64 numDisks;
    qint64 numDirs;
    qint64 numFiles;
};

#endif // SEARCHMODEL_H
