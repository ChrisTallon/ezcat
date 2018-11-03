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

#ifndef DB_H
#define DB_H

#include <QSqlDatabase>

class QWidget;

struct DBStats
{
    qint64 numCats;
    qint64 numDisks;
    qint64 numDirs;
    qint64 numFiles;
    qint64 size;
};

class DB
{
public:
    DB();

    bool getDBisOpen() const { return dbIsOpen; }
    const QString& getFileName() const { return fileName; }

    bool initLib();
    void setGUIParent(QWidget*);
    bool openDB(const QString& fileName);
    void closeDB();
    bool makeNewDB(const QString& fileName);
    bool startTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    void compact() const;
    DBStats getStats() const;
    qint64 getFileSize() const;

private:
    bool checkDBContent() const;

    QString fileName;
    QWidget* guiParent = NULL;
    QSqlDatabase db;
    bool dbIsOpen = false;
};

#endif // DB_H
