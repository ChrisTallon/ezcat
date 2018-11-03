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

#include <QStandardItemModel>

#include "globals.h"
#include "nodecatalogue.h"
#include "noderoot.h"

#include "dlgmovedisk.h"
#include "ui_movediskdialog.h"

DlgMoveDisk::DlgMoveDisk(QWidget *parent, qint64 disableSelectCat) :
    QDialog(parent), ui(new Ui::MoveDiskDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->comboBox->addItem("<none>", 0);

    qint64 indexCounter = 0;
    qint64 setIndex = -1;

    Node::getRootNode()->eachCat( [&] (NodeCatalogue* cat)
    {
        ui->comboBox->addItem(cat->getName(), cat->getID());
        ++indexCounter;
        if (cat->getID() == disableSelectCat) setIndex = indexCounter;
    });

    if (setIndex == 0) ui->comboBox->setCurrentIndex(1);
    else ui->comboBox->setCurrentIndex(0);

    if (setIndex != -1)
    {
        // Disable the item with index setIndex
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboBox->model());
        QStandardItem* item = model->item(setIndex);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }
}

DlgMoveDisk::~DlgMoveDisk()
{
    delete ui;
}

qint64 DlgMoveDisk::getSelectedCatalogue() const
{
    return ui->comboBox->currentData().toLongLong();
}
