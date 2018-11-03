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

#include <QApplication>
#include <QCommandLineParser>

#include "globals.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Command line options

    a.setApplicationName("EZ Cat");
    a.setApplicationVersion(QString("5." + QString::number(APP_VERSION)));
    QCommandLineParser qcp;
    qcp.setApplicationDescription("EZ Cat - Disk Cataloguer");
    qcp.addHelpOption();
    qcp.addVersionOption();
    QCommandLineOption openFileOption("f", "Open database file <file>.", "file");
    qcp.addOption(openFileOption);
    qcp.process(a);
    QString cliDBFile = qcp.value(openFileOption);

    // Set up globals

    if (!dbman.initLib()) return -1;
    initIcons();

    // Run

    MainWindow w(NULL, cliDBFile);
    w.show();

    return a.exec();
}
