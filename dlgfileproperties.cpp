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

#include "dfile.h"
#include "utils.h"

#include "dlgfileproperties.h"
#include "ui_filepropertiesdialog.h"

DlgFileProperties::DlgFileProperties(QWidget *t_parent, DFile* t_file) :
    QDialog(t_parent), ui(new Ui::FilePropertiesDialog), file(t_file),
    fileIcon(QIcon::fromTheme("file")),
    filePixmap(fileIcon.pixmap(100, 100))
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->labelIcon->setPixmap(filePixmap);
    //ui->lFullPath->setStyleSheet("* { background-color: rgba(0, 0, 0, 0); }");

    if (!file->loadFromDB()) Utils::errorMessageBox("Database error");
    file->loadFromFileSystem();

    if (file->getType() == TYPE_FILE)
    {
        ui->lObjectType->setText("File");
    }
    else if (file->getType() == TYPE_SYMLINK)
    {
        ui->lObjectType->setText("Symbolic Link");
    }
    else if (file->getType() == TYPE_OTHERFILEUNKNOWN)
    {
        ui->lObjectType->setText("Special File");
    }

    ui->lObjectName->setText(file->getFileName());
    ui->lObjectSize->setText(file->getSizeFullText());
    ui->lObjectModified->setText(file->getLastModifiedFullText());
    ui->lObjectOwner->setText(file->getFOwner());
    ui->lObjectGroup->setText(file->getFGroup());
    ui->lObjectPermissions->setText(file->getPermissionsText());
    ui->lObjectDisk->setText(file->getDiskName());
    ui->lObjectCat->setText(file->getCatName());

    if (file->getDiskPath().isEmpty()) ui->lFullPath->setText("/");
    else ui->lFullPath->setText(file->getDiskPath());

    if (file->isReachable())
    {
        ui->lReachable->setText("Yes");
        if (file->getType() == TYPE_FILE) ui->bOpenFile->setEnabled(true);
    }
    else
    {
        ui->lReachable->setText("No");
    }

    if (file->isContainerReachable())
    {
        ui->bOpenContainer->setEnabled(true);
    }
}

DlgFileProperties::~DlgFileProperties()
{
    delete ui;
}

void DlgFileProperties::on_bOpenFile_clicked()
{
    file->osOpen();
}

void DlgFileProperties::on_bOpenContainer_clicked()
{
    file->osOpenContainer();
}
