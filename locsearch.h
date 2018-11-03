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

#ifndef LOCSEARCH_H
#define LOCSEARCH_H

#include <QObject>
#include <QLineEdit>
#include <QIcon>

class LocSearch : public QLineEdit
{
    Q_OBJECT

public:
    LocSearch(QWidget* parent);

    int getMode() { return mode; }

    void setLocationText();
    void setLocationText(QString&);
    void clearSearch();
    void toLocationMode();
    void toResultsMode();
    enum modes { LOCATION, EDITING, RESULTS };

protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);

public slots:
    void handleReturn();

signals:
    void gotFocus();
    void lostFocus();
    void doSearch(QString text);
    void escPressed();

private:
    QIcon searchIcon;
    QString locationText;
    QString searchText;
    QAction* searchAction;
    int mode = LOCATION;
};

#endif // LOCSEARCH_H
