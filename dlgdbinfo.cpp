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

#include <QProgressDialog>
#include <QThread>
#include <QDebug>

#include "globals.h"
#include "db.h"
#include "backgroundtask.h"

#include "dlgdbinfo.h"
#include "ui_dlgdbinfo.h"

DlgDBInfo::DlgDBInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgDBInfo)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->ldbFileName->setText(db.getFileName());
    DBStats dbstats = db.getStats();
    ui->ldbFileSize->setText(fileSizeToHR(dbstats.size));
    ui->lNumCats->setText(QLocale(QLocale::English).toString(dbstats.numCats));
    ui->lNumDisks->setText(QLocale(QLocale::English).toString(dbstats.numDisks));
    ui->lNumDirs->setText(QLocale(QLocale::English).toString(dbstats.numDirs));
    ui->lNumFiles->setText(QLocale(QLocale::English).toString(dbstats.numFiles));
}

DlgDBInfo::~DlgDBInfo()
{
    delete ui;
}

void DlgDBInfo::on_bCompact_clicked()
{
    progressDialog = new QProgressDialog("Compacting database...", QString(), 0, 0, this);
    progressDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);

    QThread* thread = new QThread;
    runningCompactor = new BackgroundTask( [] { db.compact(); } );
    runningCompactor->moveToThread(thread);

    connect(thread, SIGNAL(started()), runningCompactor, SLOT(go()));
    connect(runningCompactor, SIGNAL(finished()), this, SLOT(compactorFinished()));
    connect(runningCompactor, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

void DlgDBInfo::compactorFinished()
{
    delete runningCompactor;
    delete progressDialog;
    runningCompactor = NULL;
    progressDialog = NULL;
    DBStats dbstats = db.getStats();
    ui->ldbFileSize->setText(fileSizeToHR(dbstats.size));
}
