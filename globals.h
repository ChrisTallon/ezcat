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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QIcon>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

extern QSettings settings;

#include "db.h"
extern DB dbman;

extern QIcon catalogueIcon;
extern QIcon diskIcon;
extern QIcon dirIcon;
extern QIcon fileIcon;
extern QIcon fileLinkIcon;
extern QIcon fileCogIcon;

#define APP_VERSION 0
#define DB_VERSION 0

// TableSorter relies on this ordering
const static int TYPE_INVALID = 0;
const static int TYPE_ROOT = 1;
const static int TYPE_CAT = 2;
const static int TYPE_DISK = 3;
const static int TYPE_DIR = 4;
const static int TYPE_FILE = 5;
const static int TYPE_SYMLINK = 6;
const static int TYPE_OTHERFILEUNKNOWN = 20; // pipes, devices ...

// Custom data roles for tree and table models

const static int ROLE_ID = Qt::UserRole;
const static int ROLE_TYPE = Qt::UserRole + 1;
const static int ROLE_RAW = Qt::UserRole + 2;

void initIcons();
QString fileSizeToHR(qint64 s);
QString qPermissionsToText(qint64 p);

#endif // GLOBALS_H
