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
#include <QWidget>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "globals.h"
#include "utils.h"

#include "db.h"

DB::DB()
{}

bool DB::initLib()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isValid())
    {
        Utils::errorMessageBox(guiParent, "Qt SQLite failed to initialise. Please ensure the Qt 5 SQLite 3 database driver is installed.");
        return false;
    }
    return true;
}

void DB::setGUIParent(QWidget *gp)
{
    guiParent = gp;
}

bool DB::openDB(const QString& _fileName)
{
    if (dbIsOpen) return false;
    fileName = _fileName;

    QFileInfo checkExists(fileName);
    if (!checkExists.exists())
    {
        Utils::errorMessageBox(guiParent, QString("Database file not found: ") + fileName);
        return false;
    }

    db.setDatabaseName(fileName);

    if (!db.open())
    {
        Utils::errorMessageBox(guiParent, "Failed to open database");
        return false;
    }

    if (!checkDBContent())
    {
        db.close();
        Utils::errorMessageBox(guiParent, "Database not in expected format");
        return false;
    }

    dbIsOpen = true;
    return true;
}

void DB::closeDB()
{
    if (!dbIsOpen) return;
    dbIsOpen = false;
    db.close();
    fileName.clear();
}

bool DB::makeNewDB(const QString& newFileName)
{
    QFileInfo checkExists(newFileName);
    if (checkExists.exists())
    {
        Utils::errorMessageBox(guiParent, "This file already exists! Please name a new non-existant file");
        return false;
    }

    db.setDatabaseName(newFileName);
    if (!db.open())
    {
        Utils::errorMessageBox(guiParent, "Failed to open new database file for writing");
        return false;
    }
    fileName = newFileName;

    // new file successfully opened - make a new DB

    QList<const char*> sql =
    {
#include "db-schema.txt"
#include "utils.h"
#include "utils.h"
#include "utils.h"
    };

    QSqlQuery query;

    for (qint64 i = 0; i < sql.size(); i++)
    {
        if (!query.exec(sql[i]))
        {
            Utils::errorMessageBox(guiParent, "Failed to execute schema SQL");
            dbman.closeDB();
            return false;
        }
    }

    if (!query.exec(QString("INSERT INTO ezcat_db_version (version) values (%1)").arg(DB_VERSION)))
    {
        Utils::errorMessageBox(guiParent, "Database error");
        dbman.closeDB();
        return false;
    }
    return true;
}

bool DB::checkDBContent() const
{
    QSqlQuery query;
    if (!query.exec("select * from ezcat_db_version")) return false;
    if (!query.next()) return false;
    int version = query.value(0).toInt();
    if (version != DB_VERSION) return false;
    return true;
}

bool DB::startTransaction()
{
    return db.transaction();
}

bool DB::commitTransaction()
{
    return db.commit();
}

bool DB::rollbackTransaction()
{
    return db.rollback();
}

void DB::compact() const
{
    if (!dbIsOpen) return;
    QSqlQuery query;
    query.exec(QString("vacuum"));
}

qint64 DB::getFileSize() const
{
    return QFile(fileName).size();
}

DBStats DB::getStats() const
{
    QSqlQuery query;
    struct DBStats dbstats;
    dbstats.size = dbman.getFileSize();

    query.exec(QString("select count(*) from catalogues"));
    query.next();
    dbstats.numCats = query.value(0).toLongLong();

    query.exec(QString("select count(*) from disks"));
    query.next();
    dbstats.numDisks = query.value(0).toLongLong();

    query.exec(QString("select count(*) from directories"));
    query.next();
    dbstats.numDirs = query.value(0).toLongLong();

    query.exec(QString("select count(*) from files"));
    query.next();
    dbstats.numFiles = query.value(0).toLongLong();

    return dbstats;
}
