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

#ifndef DLGFILEPROPERTIES_H
#define DLGFILEPROPERTIES_H

#include <QDialog>
#include <QIcon>
#include <QPixmap>

#include "globals.h"

namespace Ui {
class FilePropertiesDialog;
}

class DFile;

class DlgFileProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DlgFileProperties(QWidget *parent, DFile* file);
    ~DlgFileProperties();

private slots:
    void on_bOpenFile_clicked();
    void on_bOpenContainer_clicked();

private:
    Ui::FilePropertiesDialog *ui;
    DFile* file;
    QIcon fileIcon;
    QPixmap filePixmap;
};

#endif // DLGFILEPROPERTIES_H
