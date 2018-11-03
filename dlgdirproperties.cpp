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

#include "dirpropertieswidget.h"

#include "dlgdirproperties.h"
#include "ui_dlgdirproperties.h"

DlgDirProperties::DlgDirProperties(QWidget *t_parent, DDir* t_dir) :
    QDialog(t_parent),
    ui(new Ui::DlgDirProperties), dir(t_dir)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    dpw = new DirPropertiesWidget(this, dir);
    ui->centralArea->setLayout(new QVBoxLayout());
    ui->centralArea->layout()->addWidget(dpw);
}

DlgDirProperties::~DlgDirProperties()
{
    delete dpw;
    delete ui;
}
