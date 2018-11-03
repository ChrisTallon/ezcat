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

#include "globals.h"

#include "nodedir.h"

NodeDir::NodeDir(qint64 t_id, const QString& t_name, int t_accessDenied)
    : Node(TYPE_DIR, t_id, t_name)
{
    if (t_accessDenied > 0) accessDenied = true;
}

bool NodeDir::loadChildren()
{
    QSqlTableModel dirsModel;
    dirsModel.setTable("directories");
    dirsModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    dirsModel.setFilter(QString("parent = %1").arg(id));
    if (!dirsModel.select()) return false;
    while(dirsModel.canFetchMore()) dirsModel.fetchMore();

    qint64 numDirs = dirsModel.rowCount();
    for (qint64 i = 0; i < numDirs; i++)
    {
        NodeDir* newDir = new NodeDir(dirsModel.data(dirsModel.index(i, 0), Qt::DisplayRole).toLongLong(),
                                      dirsModel.data(dirsModel.index(i, 4), Qt::DisplayRole).toString(),
                                      dirsModel.data(dirsModel.index(i, 9), Qt::DisplayRole).toInt());
        addChild(newDir);

    }
    childrenLoaded = true;
    return true;
}

QString NodeDir::summaryText() const
{
    QString text = "Directory: " + name + ". " + dirStats(id) + ".";
    return text;
}
