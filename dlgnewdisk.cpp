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
#include <QFileDialog>
#include <QMessageBox>
#include <QStorageInfo>

#include "globals.h"
#include "nodecatalogue.h"
#include "noderoot.h"
#include "utils.h"

#include "dlgnewdisk.h"
#include "ui_newdiskdialog.h"

DlgNewDisk::DlgNewDisk(QWidget *parent, qint64 preSelectCat) :
    QDialog(parent), ui(new Ui::NewDiskDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->editNewDiskName->setFocus();

    ui->comboBox->addItem("<none>", 0);

    qint64 indexCounter = 0;
    qint64 setIndex = 0;
    Node::getRootNode()->eachCat( [&] (NodeCatalogue* cat) {
        ui->comboBox->addItem(cat->getName(), cat->getID());
        ++indexCounter;
        if (cat->getID() == preSelectCat) setIndex = indexCounter;
    });
    ui->comboBox->setCurrentIndex(setIndex);
}

DlgNewDisk::~DlgNewDisk()
{
    delete ui;
}

void DlgNewDisk::updateMode(const QString &diskName, const QString &catPath)
{
    setWindowTitle("Update Disk");
    ui->lNewDiskName->setText("Disk name:");
    ui->lCatalogue->setText("Put disk in catalogue:");
    ui->editNewDiskName->setText(diskName);
    ui->editLocation->setText(catPath);
}

qint64 DlgNewDisk::getSelectedCatalogue() const
{
    return ui->comboBox->currentData().toLongLong();
}

QString DlgNewDisk::getNewDiskName() const
{
    return ui->editNewDiskName->text();
}

QString DlgNewDisk::getScanLocation() const
{
    return ui->editLocation->text();
}

void DlgNewDisk::on_chooseLocation_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                NULL,
                "Select location to catalogue",
                NULL,
                NULL,
                NULL,
                QFileDialog::Options(QFileDialog::ShowDirsOnly)
            );

    if (!fileName.isEmpty())
    {
        ui->editLocation->setText(fileName);

        if (ui->editNewDiskName->text().isEmpty())
        {
            QDir qd(fileName);
            QStorageInfo storageInfo(qd);

            if (    (fileName == storageInfo.rootPath())
                 && (!storageInfo.name().isEmpty()))
                ui->editNewDiskName->setText(storageInfo.name());
            else
                ui->editNewDiskName->setText(qd.dirName());
        }
    }
}

void DlgNewDisk::done(int code)
{
    if (code == QDialog::Accepted)
    {
        if (ui->editNewDiskName->text().size() == 0)
        {
            Utils::errorMessageBox(this, "Please enter a name for the new disk");
            return;
        }
        else if (ui->editLocation->text().size() == 0)
        {
            Utils::errorMessageBox(this, "Please enter a location to catalogue");
            return;
        }
        else
        {
            QFileInfo target(ui->editLocation->text());

            if (!target.exists())
            {
                Utils::errorMessageBox(this, "Location to catalogue does not exist");
                return;
            }

            if (!target.isDir())
            {
                Utils::errorMessageBox(this, "Location to catalogue is not a directory");
                return;
            }

            if (QDir(ui->editLocation->text()).isEmpty())
            {
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Location is empty");
                msgBox.setText("Location to catalogue is empty. Continue?");
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                int ret = msgBox.exec();

                if (ret == QMessageBox::Yes)
                {
                    QDialog::done(code);
                    return;
                }

                return;
            }

            QDialog::done(code);
            return;
        }
    }
    else // user cancelled, just return
    {
        QDialog::done(code);
        return;
    }
}
