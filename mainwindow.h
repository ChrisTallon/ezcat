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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QModelIndex>
#include <QMainWindow>
#include <QIcon>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class TreeModel;
class Node;
class NodeCatalogue;
class NodeDisk;
class DFile;
class DDir;
class TableModel;
class Cataloguer;
class BackgroundTask;
class QProgressDialog;
class QSortFilterProxyModel;
class SearchModel;
class SearchResult;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent, const QString& cliDBFile);
    ~MainWindow();

public slots:
    void updateCataloguerProgress(qint64 numObjects);
    void cataloguerFinished(NodeDisk* newDisk);
    void updateCataloguerReindexing();

private slots:
    void on_actionQuit_triggered();
    void on_buttonGoUp_clicked();
    void on_actionDelete_triggered();
    void on_actionSearch_triggered();
    void on_actionRename_triggered();
    void on_actionDatabaseNew_triggered();
    void on_actionDatabaseLoad_triggered();
    void on_actionDatabaseClose_triggered();
    void on_actionDatabaseProperties_triggered();
    void on_actionCatalogueNew_triggered();
    void on_actionCatalogueDelete_triggered();
    void on_actionCatalogueRename_triggered();
    void on_actionDiskNew_triggered(bool updateMode = false);
    void on_actionDiskMount_triggered();
    void on_actionDiskUnmount_triggered();
    void on_actionDiskUpdate_triggered();
    void on_actionDiskRename_triggered();
    void on_actionDiskMove_triggered();
    void on_actionDiskProperties_triggered();
    void on_actionDiskDelete_triggered();
    void on_actionFileOpen_triggered();
    void on_actionDirOpen_triggered();
    void on_actionFileOpenContaining_triggered();
    void on_actionFileProperties_triggered();
    void on_actionDirGo_triggered();
    void on_actionDirProperties_triggered();
    void on_splitter_splitterMoved(int pos, int index);
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionCXONLYDiskGo_triggered();
    void on_actionDiskOpen_triggered();
    void on_actionCxtSearchResultGo_triggered();
    void on_actionCxtSearchResultOpen_triggered();
    void on_actionCxtSearchResultOpenContaining_triggered();
    void on_actionCxtSearchResultProperties_triggered();
    void on_treeView_clicked(const QModelIndex &index);
    void on_tableView_activated(const QModelIndex &index);

    void treeView_current_changed(const QModelIndex&, const QModelIndex&);
    void treeView_collapsed(const QModelIndex &sQmi);
    void treeView_dataChanged(const QModelIndex&, const QModelIndex&);
    void treeViewContextMenu(QPoint qp);
    void tableViewContextMenu(QPoint qp);
    void tableView_ddf_current_changed(const QModelIndex&, const QModelIndex&);
    void tableView_s_current_changed(const QModelIndex&, const QModelIndex&);
    void focusChanged(QWidget* old, QWidget* now);
    void cataloguerAbort();
    void handleDoSearch(QString);
    void handleLocSearchGotFocus();
    void handleLocSearchLostFocus();
    void handleLocSearchEsc();
    void backgroundTaskFinished();
    void catalogueDeleteFinsihed(NodeCatalogue*, bool);
    void diskDeleteFinsihed(NodeDisk*, bool);
    void requestRenameDisk(qint64 diskID, QString newName);

private:
    Ui::MainWindow *ui;
    QIcon iconGoUp;
    QIcon iconGoPrevious;
    TreeModel* tm = NULL;
    QSortFilterProxyModel* tms = NULL;
    TableModel* fm = NULL;
    QSortFilterProxyModel* fms = NULL;
    Cataloguer* runningCataloguer = NULL;
    QProgressDialog* progressDialog = NULL;
    BackgroundTask* backgroundTask = NULL;
    SearchModel* searchModel = NULL;
    QLabel statusLabel;
    bool statusLabelHold = false;
    DFile* tableSelectedFile = NULL;
    DDir* tableSelectedDir = NULL;
    DDir* treeSelectedDir = NULL;
    NodeDisk* tableSelectedDisk = NULL;
    SearchResult* searchCurrentResult = NULL;

    bool getConfigDatabase();
    void tableToFilesDirs();
    void setLocationText();
    Node* getCurrentTreeItem() const;
    void databaseJustOpened();
    void returnFromSearch();
    void addNewDiskToAll(NodeDisk *disk);
    void dfDirGo(qint64 targetID);
    void setActions();
    bool allowDiskMove();
    bool allowDiskMount();
    bool allowDiskUnmount();
    bool allowDirOpen(int uiSource);
    bool allowFileOpen();
    bool allowFileOpenContaining();
    bool allowDiskOpen();
    NodeDisk* getCurrentDisk(QModelIndex* usQmi) const;
    void navigateToSearchResult();
    bool dfSelectedDirAccessCheck();
    void clearDataIfLast();

protected:
    virtual void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
