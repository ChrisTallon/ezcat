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

#ifndef DLGNEWDISK_H
#define DLGNEWDISK_H

#include <QDialog>

namespace Ui {
class NewDiskDialog;
}

class DlgNewDisk : public QDialog
{
    Q_OBJECT

public:
    explicit DlgNewDisk(QWidget *parent, qint64 preSelectCat);
    ~DlgNewDisk();
    void updateMode(const QString& diskName, const QString& catPath);
    qint64 getSelectedCatalogue() const;
    QString getNewDiskName() const;
    QString getScanLocation() const;

protected:
    void done(int code);

private slots:
    void on_chooseLocation_clicked();

private:
    Ui::NewDiskDialog *ui;
};

#endif // DLGNEWDISK_H
