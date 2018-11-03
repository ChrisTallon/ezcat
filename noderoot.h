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

#ifndef NODEROOT_H
#define NODEROOT_H

#include "node.h"

class NodeCatalogue;
class NodeDisk;

class NodeRoot : public Node
{
public:
    NodeRoot();
    virtual bool loadChildren();
    qint64 numCatalogues() const { return numCats; }

    virtual QString summaryText() const;
    void eachCat(std::function<void (NodeCatalogue *)> func);
    NodeCatalogue* catFromID(qint64 id) const;
    NodeDisk* diskFromID(qint64 id) const;
    NodeDisk* diskFromIDRecursive(qint64 id) const;

private:
    qint64 numCats = 0;
    qint64 numDisks = 0;

};

#endif // NODEROOT_H
