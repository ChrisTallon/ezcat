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

#include "ddir.h"

#include "dirpropertieswidget.h"
#include "ui_dirpropertieswidget.h"

DirPropertiesWidget::DirPropertiesWidget(QWidget *t_parent, DDir* t_dir) :
    QWidget(t_parent),
    ui(new Ui::DirPropertiesWidget),
    dirIcon(QIcon::fromTheme("folder")),
    dirPixmap(dirIcon.pixmap(100, 100)),
    dir(t_dir)
{
    ui->setupUi(this);

    ui->lIcon->setPixmap(dirPixmap);

    ui->editDirName->setText(dir->getDirName());
    ui->editDiskPath->setText(dir->getDiskPath());
    ui->lContents->setText(dir->getDirStats());
    ui->lOwner->setText(dir->getFOwner());
    ui->lGroup->setText(dir->getFGroup());
    ui->lPermissions->setText(dir->getPermissionsText());
    ui->lModified->setText(dir->getLastModifiedFullText());
    ui->lDiskName->setText(dir->getDiskName());
    ui->lCatalogue->setText(dir->getCatName());

    if (dir->isReachable())
    {
        ui->lReachable->setText("Yes");
        ui->bOpenDirectory->setEnabled(true);
    }
    else
    {
        ui->lReachable->setText("No");
    }
}

DirPropertiesWidget::~DirPropertiesWidget()
{
    delete ui;
}

void DirPropertiesWidget::on_bOpenDirectory_clicked()
{
    dir->osOpen();
}

void DirPropertiesWidget::on_bCalcSubContents_clicked()
{
    QLabel* label = new QLabel(this);
    label->setText("Wait...");

    ui->formLayout->replaceWidget(ui->bCalcSubContents, label);
    ui->bCalcSubContents->hide();
    QApplication::processEvents();

    label->setText(dir->getSubContents());
}
