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

#include <QDateTime>
#include <QDebug>

#include "globals.h"
#include "nodecatalogue.h"
#include "nodedisk.h"
#include "ddir.h"
#include "utils.h"
#include "dirpropertieswidget.h"

#include "dlgdiskdirproperties.h"
#include "ui_diskdirproperties.h"

DlgDiskDirProperties::DlgDiskDirProperties(QWidget *t_parent, NodeDisk* t_disk) :
    QDialog(t_parent), ui(new Ui::DiskDirProperties), disk(t_disk),
    diskIcon(QIcon::fromTheme("media-floppy")),
    diskPixmap(diskIcon.pixmap(100, 100))
{
    ui->setupUi(this);

    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->lIcon->setPixmap(diskPixmap);

    QString fsSize = fileSizeToHR(disk->getFSSize());
    fsSize += " (";
    fsSize += QLocale(QLocale::English).toString(disk->getFSSize());
    fsSize += " B)";

    QString fsFree = fileSizeToHR(disk->getFSFree());
    fsFree += " (";
    fsFree += QLocale(QLocale::English).toString(disk->getFSFree());
    fsFree += " B)";

    ui->editDiskName->setText(disk->getName());

    NodeCatalogue* cat = static_cast<NodeCatalogue*>(disk->getParent());
    if (cat)
        ui->lCatalogue->setText(cat->getName());
    else
        ui->lCatalogue->setText("<none>");

    ui->editCatPath->setText(disk->getCatPath());
    ui->lDeviceName->setText(disk->getDeviceName());
    ui->lFSLabel->setText(disk->getFSLabel());
    ui->lFSUUID->setText(disk->getUuid());

    QDateTime qdt;
    qdt.setSecsSinceEpoch(disk->getCatTime());
    ui->lCatTime->setText(qdt.toString("dddd, d MMMM yyyy hh:mm:ss t"));

    ui->lTotalSize->setText(fsSize);
    ui->lFreeSpace->setText(fsFree);
    ui->lFSType->setText(disk->getFSType());
    ui->editMountCommand->setText(disk->getMountCommand());
    ui->editUnmountCommand->setText(disk->getUnmountCommand());

    // ---

    ddir = new DDir(disk->getRootDirID());
    if (!ddir->loadFromDB()) Utils::errorMessageBox("Database error");
    ddir->loadFromFileSystem();

    dpw = new DirPropertiesWidget(this, ddir);
    ui->tab2CentralArea->setLayout(new QVBoxLayout());
    ui->tab2CentralArea->layout()->addWidget(dpw);
}

DlgDiskDirProperties::~DlgDiskDirProperties()
{
    delete ui;
    delete ddir;
}

void DlgDiskDirProperties::done(int code)
{
    if (code == QDialog::Accepted)
    {
        QString newMountCommand = ui->editMountCommand->text();
        QString newUnmountCommand = ui->editUnmountCommand->text();
        disk->setCommands(newMountCommand, newUnmountCommand);
    }

    QDialog::done(code);
}
