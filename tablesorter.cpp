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

#include "globals.h"

#include "tablesorter.h"

TableSorter::TableSorter()
{}

bool TableSorter::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int sortCol = sortColumn();
    auto model = sourceModel();

    int leftType = model->data(source_left, ROLE_TYPE).toInt();
    int rightType = model->data(source_right, ROLE_TYPE).toInt();

    Qt::SortOrder sortOrd = sortOrder();

    if (sortOrd == Qt::AscendingOrder)
    {
        if ((leftType == TYPE_DIR) && (rightType >= TYPE_FILE)) return true;
        if ((leftType >= TYPE_FILE) && (rightType == TYPE_DIR)) return false;
    }
    else // (sortOrd == Qt::DescendingOrder)
    {
        if ((leftType == TYPE_DIR) && (rightType >= TYPE_FILE)) return false; // invert these to keep dirs on top even in descending order sorting
        if ((leftType >= TYPE_FILE) && (rightType == TYPE_DIR)) return true;
    }

    if (sortCol == 2) // Override size to sort by raw data instead of cooked size values
    {
        QVariant L = model->data(source_left, ROLE_RAW);
        QVariant R = model->data(source_right, ROLE_RAW);
        if ((L == QVariant()) || (R == QVariant())) return false;
        return L < R;
    }
    else if (sortCol == 3) // Override modified to sort by timestamp rather than date string
    {
        QVariant L = model->data(source_left, ROLE_RAW);
        QVariant R = model->data(source_right, ROLE_RAW);
        return L < R;
    }
    else
    {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}
