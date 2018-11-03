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

#ifndef NODEDISK_H
#define NODEDISK_H

#include <QDir>
#include <QObject>
#include <QString>

#include "node.h"

class QProcess;

class NodeDisk : public QObject, public Node
{
    Q_OBJECT

public:
    NodeDisk(qint64 id, qint64 catID, const QString& name, const QString& catPath,
         qint64 catTime, const QString& deviceName, const QString& fsLabel,
         const QString& fsType, qint64 fsSize, qint64 fsFree, int isRoot,
         const QString& mountCommand, const QString& unmountCommand, const QString& uuid);
    bool loadRootDirID();
    virtual bool loadChildren();

    qint64 getCatID() const { return catID; }
    const QString& getCatPath() const { return catPath; }
    qint64 getCatTime() const { return catTime; }
    const QString& getFSLabel() const { return fsLabel; }
    const QString& getFSType() const { return fsType; }
    qint64 getFSSize() const { return fsSize; }
    qint64 getFSFree() const { return fsFree; }
    const QString& getMountCommand() const { return mountCommand; }
    const QString& getUnmountCommand() const { return unmountCommand; }
    const QString& getUuid() const { return uuid; }
    const QString& getDeviceName() const { return deviceName; }
    qint64 getRootDirID() const { return rootDirID; }

    virtual QString summaryText() const;
    void removeFromDB();
    bool removeContentsFromDBNT() const;
    bool moveToCatalogue(qint64 newCat);
    bool rename(const QString& newName);
    void setCommands(const QString &newMountCommand, const QString &newUnmountCommand);
    bool hasMountCommand() const;
    bool catPathLocationEmpty() const;
    bool hasUnmountCommand() const;
    bool catPathLocationFull() const;
    bool mount();
    bool unmount();
    bool isReachable();
    void osOpen() const;
    void update(qint64 catID, const QString& name, const QString& catPath,
             qint64 catTime, const QString& deviceName, const QString& fsLabel,
             const QString& fsType, qint64 fsSize, qint64 fsFree, int isRoot, const QString& uuid);

    static NodeDisk* createDisk(qint64 catID, const QString &name, const QString &catPath,
                                const QString& deviceName, const QString& fsLabel,
                                const QString &fsType, qint64 fsSize, qint64 fsFree, int isRoot, const QString& uuid);

private:
    qint64 catID = -1;
    QString catPath;
    qint64 catTime;
    QString deviceName;
    QString fsLabel;
    QString fsType;
    qint64 fsSize;
    qint64 fsFree;
    int isRoot;
    QString mountCommand;
    QString unmountCommand;
    QString uuid;
    qint64 rootDirID = -1;
    bool fsLoaded = false;
    QDir fsQDir;

    bool startProcess(const QString &program, const QStringList &arguments);
    void loadFromFileSystem();

signals:
    void deleteFinished(NodeDisk*, bool);
};

#endif // NODEDISK_H
