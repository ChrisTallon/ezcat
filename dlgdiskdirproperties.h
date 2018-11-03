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

#ifndef DLGDISKDIRPROPERTIES_H
#define DLGDISKDIRPROPERTIES_H

#include <QDialog>
#include <QIcon>

namespace Ui {
class DiskDirProperties;
}

class NodeDisk;
class DirPropertiesWidget;
class DDir;

class DlgDiskDirProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DlgDiskDirProperties(QWidget *parent, NodeDisk* disk);
    ~DlgDiskDirProperties();

private:
    Ui::DiskDirProperties *ui;
    NodeDisk* disk;
    QIcon diskIcon;
    QPixmap diskPixmap;
    DirPropertiesWidget* dpw;
    DDir* ddir;

public slots:
    void done(int);
};

#endif // DLGDISKDIRPROPERTIES_H
