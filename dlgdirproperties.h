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

#ifndef DLGDIRPROPERTIES_H
#define DLGDIRPROPERTIES_H

#include <QDialog>

namespace Ui {
class DlgDirProperties;
}

class DirPropertiesWidget;
class DDir;

class DlgDirProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DlgDirProperties(QWidget *parent, DDir* dir);
    ~DlgDirProperties();

private:
    Ui::DlgDirProperties *ui;
    DirPropertiesWidget* dpw;
    DDir* dir;
};

#endif // DLGDIRPROPERTIES_H
