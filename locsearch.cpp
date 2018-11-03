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
#include <QIcon>
#include <QKeyEvent>

#include "globals.h"

#include "locsearch.h"

LocSearch::LocSearch(QWidget* parent)
    : QLineEdit(parent)
{
    searchIcon = QIcon::fromTheme("edit-find");
    connect(this, SIGNAL(returnPressed()), this, SLOT(handleReturn()));
    setPlaceholderText("Enter search text...");
}

void LocSearch::mousePressEvent(QMouseEvent* /*event*/)
{
    // Supressing the default behaviour allows keeping the text selected
    // QLineEdit::mousePressEvent(event);
}

void LocSearch::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);

    if (mode == LOCATION) searchAction = addAction(searchIcon, QLineEdit::LeadingPosition);
    mode = EDITING;
    setText(searchText);
    setClearButtonEnabled(true);
    setSelection(0, searchText.size());

    emit gotFocus();
}

void LocSearch::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    emit lostFocus();
}

void LocSearch::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        emit escPressed();
        return;
    }

    QLineEdit::keyPressEvent(event);
}

void LocSearch::handleReturn()
{
    searchText = text();
    toResultsMode();
    emit doSearch(searchText);
}

void LocSearch::setLocationText()
{
    locationText = QString();
    if (mode == LOCATION) setText(locationText); // Not sure if this could ever happen
}

void LocSearch::setLocationText(QString& _locationText)
{
    locationText = _locationText;
    if (mode == LOCATION) setText(locationText);
}

void LocSearch::clearSearch()
{
    mode = LOCATION;
    searchText .clear();
    setClearButtonEnabled(false);
    setText(locationText);
    removeAction(searchAction);
}

void LocSearch::toLocationMode()
{
    mode = LOCATION;
    setClearButtonEnabled(false);
    removeAction(searchAction);
    setText(locationText);
}

void LocSearch::toResultsMode()
{
    mode = RESULTS;
    setClearButtonEnabled(false);
    setText(QString("Search results for: ") + searchText);
}
