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

#ifndef NODEDIR_H
#define NODEDIR_H

#include "node.h"

class NodeDir : public Node
{
public:
    NodeDir(qint64 id, const QString& name, int t_accessDenied);
    bool isAccessDenied() const { return accessDenied; }
    virtual bool loadChildren();
    virtual QString summaryText() const;

private:
    bool accessDenied = false;
};

#endif // NODEDIR_H
