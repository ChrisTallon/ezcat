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

#ifndef DLGDBINFO_H
#define DLGDBINFO_H

#include <QDialog>

namespace Ui {
class DlgDBInfo;
}

class QProgressDialog;
class BackgroundTask;

class DlgDBInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DlgDBInfo(QWidget *parent = 0);
    ~DlgDBInfo();

private slots:
    void on_bCompact_clicked();
    void compactorFinished();

private:
    Ui::DlgDBInfo *ui;
    QProgressDialog* progressDialog = NULL;
    BackgroundTask* runningCompactor = NULL;

};

#endif // DLGDBINFO_H
