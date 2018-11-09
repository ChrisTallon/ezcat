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
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "globals.h"
#include "utils.h"

#include "db.h"

QString DB::fileName;

DB::DB()
{
}

DB::~DB()
{
    bool removeDB;
    QString conName;

    if (secondary)
    {
        // As per removedatabase docs, remove db connection after all DB objects have gone out of scope
        removeDB = qdp->isValid();
        if (removeDB) conName = qdp->connectionName();
    }

    delete qdp; // Force destruction of QSqlDatabase here

    // Only do this for secondary connections. removing the default connection causes a segfault
    if (secondary && removeDB) if (removeDB) QSqlDatabase::removeDatabase(conName);
}

bool DB::initLib(const QString& secondaryName)
{
    /* Would prefer to have the QSqlDatabase object as a class member, but it has to be destroyed
     * before calling removeDatabase(), so it is moved to the heap here
     */

    if (secondaryName.isEmpty())
    {
        qdp = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    }
    else
    {
        secondary = true;
        qdp = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", secondaryName));
    }

    if (!qdp->isValid())
    {
        Utils::errorMessageBox("Qt SQLite failed to initialise. Please ensure the Qt 5 SQLite 3 database driver is installed.");
        return false;
    }
    return true;
}

QSqlDatabase& DB::getqdb()
{
    return *qdp;
}

bool DB::openDB(const QString& _fileName)
{
    if (dbIsOpen) return false;
    fileName = _fileName;

    QFileInfo checkExists(fileName);
    if (!checkExists.exists())
    {
        Utils::errorMessageBox(QString("Database file not found: ") + fileName);
        return false;
    }

    qdp->setDatabaseName(fileName);

    if (!qdp->open())
    {
        Utils::errorMessageBox("Failed to open database");
        return false;
    }

    if (!checkDBContent())
    {
        qdp->close();
        Utils::errorMessageBox("Database not in expected format");
        return false;
    }

    dbIsOpen = true;
    return true;
}

bool DB::openDB() // Secondary connections. Uses stored static fileName
{
    if (dbIsOpen) return false;

    qdp->setDatabaseName(fileName);

    if (!qdp->open())
    {
        Utils::errorMessageBox("Failed to open database");
        return false;
    }

    dbIsOpen = true;
    return true;
}

void DB::closeDB()
{
    if (!dbIsOpen) return;
    dbIsOpen = false;
    qdp->close();
    if (!secondary) fileName.clear();
}

bool DB::makeNewDB(const QString& newFileName)
{
    QFileInfo checkExists(newFileName);
    if (checkExists.exists())
    {
        Utils::errorMessageBox("This file already exists! Please name a new non-existant file");
        return false;
    }

    qdp->setDatabaseName(newFileName);
    if (!qdp->open())
    {
        Utils::errorMessageBox("Failed to open new database file for writing");
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
            Utils::errorMessageBox("Failed to execute schema SQL");
            closeDB();
            return false;
        }
    }

    if (!query.exec(QString("INSERT INTO ezcat_db_version (version) values (%1)").arg(DB_VERSION)))
    {
        Utils::errorMessageBox("Database error");
        closeDB();
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
    return qdp->transaction();
}

bool DB::commitTransaction()
{
    return qdp->commit();
}

bool DB::rollbackTransaction()
{
    return qdp->rollback();
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
    dbstats.size = getFileSize();

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
