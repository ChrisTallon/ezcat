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

#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QSettings>
#include <QLocale>
#include <QFileDevice>

#include "db.h"

#include "globals.h"

QSettings settings("Loggytronic", "ezcat");
DB db;

QIcon catalogueIcon;
QIcon diskIcon;
QIcon dirIcon;
QIcon fileIcon;
QIcon fileLinkIcon;
QIcon fileCogIcon;

void initIcons()
{
    QStyle *currentStyle = QApplication::style();
    catalogueIcon = currentStyle->standardPixmap(QStyle::SP_DriveHDIcon);
    diskIcon = currentStyle->standardPixmap(QStyle::SP_DriveFDIcon);
    dirIcon = currentStyle->standardPixmap(QStyle::SP_DirIcon);
    fileIcon = currentStyle->standardPixmap(QStyle::SP_FileIcon);
    fileLinkIcon = currentStyle->standardPixmap(QStyle::SP_FileLinkIcon);
    fileCogIcon = currentStyle->standardPixmap(QStyle::SP_FileDialogDetailedView); // Find something better for this
}

QString fileSizeToHR(qint64 s)
{
    if (s < 1024) return QLocale(QLocale::English).toString(s) + " B";

    if (s < 1048576)
    {
        double d = s / 1024.0;
        return QLocale(QLocale::English).toString(d, 'f', 1) + " KiB";
    }

    if (s < 1073741824)
    {
        double d = s / 1048576.0;
        return QLocale(QLocale::English).toString(d, 'f', 1) + " MiB";
    }

    if (s < 1099511627776)
    {
        double d = s / 1073741824.0;
        return QLocale(QLocale::English).toString(d, 'f', 1) + " GiB";
    }

    double d = s / 1099511627776.0;
    return QLocale(QLocale::English).toString(d, 'f', 1) + " TiB";

    return QString();
}

QString qPermissionsToText(qint64 p)
{
    QString ptext("---------");

    if(p & QFileDevice::ReadUser) ptext[0] = 'r';
    if(p & QFileDevice::WriteUser) ptext[1] = 'w';
    if(p & QFileDevice::ExeUser) ptext[2] = 'x';
    if(p & QFileDevice::ReadGroup) ptext[3] = 'r';
    if(p & QFileDevice::WriteGroup) ptext[4] = 'w';
    if(p & QFileDevice::ExeGroup) ptext[5] = 'x';
    if(p & QFileDevice::ReadOther) ptext[6] = 'r';
    if(p & QFileDevice::WriteOther) ptext[7] = 'w';
    if(p & QFileDevice::ExeOther) ptext[8] = 'x';
    return QString(ptext);
}
