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

#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QList>
#include <QInputDialog>
#include <QDateTime>
#include <QProgressDialog>
#include <QThread>
#include <QSortFilterProxyModel>
#include <QTimer>

#include "globals.h"
#include "locsearch.h"
#include "ui_mainwindow.h"
#include "treemodel.h"
#include "tablemodel.h"
#include "tablesorter.h"
#include "dlgnewdisk.h"
#include "cataloguer.h"
#include "backgroundtask.h"
#include "nodecatalogue.h"
#include "dlgdbinfo.h"
#include "noderoot.h"
#include "nodedisk.h"
#include "dfile.h"
#include "ddir.h"
#include "dlgmovedisk.h"
#include "dlgfileproperties.h"
#include "searchmodel.h"
#include "searchresult.h"
#include "dlgdiskdirproperties.h"
#include "dlgdirproperties.h"
#include "dlgabout.h"
#include "dlgaccessdenieds.h"
#include "nodedir.h"
#include "utils.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent, const QString& cliDBFile) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    dbman.setGUIParent(this);

    iconGoUp = QIcon::fromTheme("go-up");
    iconGoPrevious = QIcon::fromTheme("go-previous");

    QList<int> sizes;
    sizes << 60 << 210;
    ui->splitter->setSizes(sizes);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setShowGrid(false);

    ui->statusBar->addWidget(&statusLabel);

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)));

    // set up tree view

    // FIXME - seems like there should be a better way to do this
    // The default treeview behaviour is to elide text
    // The following line resizes to contents but doesn't look too good when the content is thinner than the treeview
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // So below is a signal/slot from the QSplitter to manually resize the column when the splitter is moved
    // Also there is a large default minimum size in designer.


    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);
    connect(ui->treeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(treeView_collapsed(const QModelIndex&)));

    // set up table view
    QHeaderView* verticalHeader = ui->tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(22);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setEditTriggers(QAbstractItemView::EditKeyPressed);

    connect(ui->locSearch, SIGNAL(doSearch(QString)), this, SLOT(handleDoSearch(QString)));
    connect(ui->locSearch, SIGNAL(gotFocus()), this, SLOT(handleLocSearchGotFocus()));
    connect(ui->locSearch, SIGNAL(lostFocus()), this, SLOT(handleLocSearchLostFocus()));
    connect(ui->locSearch, SIGNAL(escPressed()), this, SLOT(handleLocSearchEsc()));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeViewContextMenu(QPoint)));

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableViewContextMenu(QPoint)));

    if (!cliDBFile.isEmpty())
    {
        if (dbman.openDB(cliDBFile))
        {
            databaseJustOpened();
        }
    }
    else if (getConfigDatabase())
    {
        databaseJustOpened();
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Error");
        msgBox.setText("No default database. Create new or open?");
        msgBox.setIcon(QMessageBox::Question);
        QAbstractButton* newButton = msgBox.addButton("Create new", QMessageBox::ActionRole);
        QAbstractButton* loadButton = msgBox.addButton("Open...", QMessageBox::AcceptRole);
        msgBox.addButton("Cancel", QMessageBox::RejectRole);
        msgBox.exec();
        QAbstractButton* clickedButton = msgBox.clickedButton();
        if (clickedButton == newButton) on_actionDatabaseNew_triggered();
        if (clickedButton == loadButton) on_actionDatabaseLoad_triggered();
    }
}

MainWindow::~MainWindow()
{
    if (tableSelectedFile) delete tableSelectedFile;
    if (tableSelectedDir) delete tableSelectedDir;
    if (treeSelectedDir) delete treeSelectedDir;
    delete ui;
    delete tms;
    delete tm;
    delete fms;
    delete fm;
    dbman.closeDB();
}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
    if (searchModel)
    {
        settings.setValue("scolwidth0", ui->tableView->columnWidth(0) < 100 ? 100 : ui->tableView->columnWidth(0));
        settings.setValue("scolwidth1", ui->tableView->columnWidth(1) < 100 ? 100 : ui->tableView->columnWidth(1));
    }
    else if (fms) // If !fms here then no DB is open, there are no columns to read
    {
        settings.setValue("dfcolwidth0", ui->tableView->columnWidth(0) < 50 ? 50 : ui->tableView->columnWidth(0));
        settings.setValue("dfcolwidth1", ui->tableView->columnWidth(1) < 50 ? 50 : ui->tableView->columnWidth(1));
        settings.setValue("dfcolwidth2", ui->tableView->columnWidth(2) < 50 ? 50 : ui->tableView->columnWidth(2));
        settings.setValue("dfcolwidth3", ui->tableView->columnWidth(3) < 50 ? 50 : ui->tableView->columnWidth(3));
        settings.setValue("dfcolwidth4", ui->tableView->columnWidth(4) < 50 ? 50 : ui->tableView->columnWidth(4));
        settings.setValue("dfcolwidth5", ui->tableView->columnWidth(5) < 50 ? 50 : ui->tableView->columnWidth(5));
    }
}

void MainWindow::on_actionDatabaseNew_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "New Database File", "", "EZ Cat Database (*.db);;All Files (*)");
    if (fileName.isEmpty()) return;
    if (dbman.getDBisOpen()) on_actionDatabaseClose_triggered();
    if (dbman.makeNewDB(fileName))
    {
        settings.setValue("dbfile", fileName);
        databaseJustOpened();
    }
}

void MainWindow::on_actionDatabaseLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Database File", "", "EZ Cat Database (*.db);;All Files (*)");
    if (fileName.isEmpty()) return;
    if (dbman.getDBisOpen()) on_actionDatabaseClose_triggered();
    if (dbman.openDB(fileName))
    {
        settings.setValue("dbfile", fileName);
        databaseJustOpened();
    }
}

void MainWindow::on_actionSearch_triggered()
{
    ui->locSearch->setFocus();
}

void MainWindow::on_actionDatabaseProperties_triggered()
{
    DlgDBInfo dbid(this);
    dbid.exec();
}

void MainWindow::on_actionDatabaseClose_triggered()
{
    if (tableSelectedFile) { delete tableSelectedFile; tableSelectedFile = NULL; }
    if (tableSelectedDir) { delete tableSelectedDir; tableSelectedDir = NULL; }
    if (treeSelectedDir) { delete treeSelectedDir; treeSelectedDir = NULL; }
    if (tableSelectedDisk) { tableSelectedDisk = NULL; }

    QItemSelectionModel *m = ui->tableView->selectionModel();    // http://doc.qt.io/qt-5/qabstractitemview.html#setModel
    ui->tableView->setModel(NULL);
    delete m;
    m = ui->treeView->selectionModel();    // http://doc.qt.io/qt-5/qabstractitemview.html#setModel
    ui->treeView->setModel(NULL);
    delete m;
    delete tms;
    delete tm;
    delete fms;
    delete fm;
    tms = NULL;
    tm = NULL;
    fms = NULL;
    fm = NULL;
    dbman.closeDB();
    ui->locSearch->setLocationText();
    ui->actionDatabaseClose->setEnabled(false);
    ui->actionDatabaseProperties->setEnabled(false);
    ui->actionSearch->setEnabled(false);
    ui->actionCatalogueNew->setEnabled(false);
    ui->actionDiskNew->setEnabled(false);
    treeView_current_changed(QModelIndex(), QModelIndex()); // Not really, but it will disable all the right GUI parts
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionCatalogueNew_triggered()
{
    QInputDialog qid(this);
    qid.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    qid.setWindowFlag(Qt::WindowMaximizeButtonHint, false); // ignored.
    qid.setWindowTitle("New Catalogue");
    qid.setLabelText("Enter a name for the new catalogue:");

    if (qid.exec() == QDialog::Accepted)
    {
        QString text = qid.textValue();

        if (!text.isEmpty())
        {
            NodeCatalogue* newCat = NodeCatalogue::createCatalogue(text);
            tm->addCatalogue(newCat);
            ui->statusBar->clearMessage();

            // If there is now more than one catalogue and the current item is a disk, enable the disk move action
            if (Node::getRootNode()->numCatalogues() > 0)
            {
                Node* t = getCurrentTreeItem();
                if (t && (t->getType() == TYPE_DISK)) ui->actionDiskMove->setEnabled(true);
            }
        }
    }
}

void MainWindow::on_actionCatalogueRename_triggered()
{
    ui->treeView->edit(ui->treeView->currentIndex());
}

void MainWindow::on_actionCatalogueDelete_triggered()
{
    QModelIndex usQmi = tms->mapToSource(ui->treeView->currentIndex());
    Node* n = static_cast<Node*>(usQmi.internalPointer());
    Q_ASSERT(n->getType() == TYPE_CAT);
    QMessageBox msgBox;
    msgBox.setText(QString("Are you sure you want to delete the catalogue: '") + n->getName() + "'?");
    msgBox.setInformativeText("All disks contained in this catalogue will also be deleted. This cannot be un-done.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    if (msgBox.exec() == QMessageBox::Ok)
    {
        progressDialog = new QProgressDialog("Deleting catalogue...", QString(), 0, 0, this);
        progressDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setMinimumDuration(500);
        progressDialog->setValue(0);

        NodeCatalogue* catToDel = static_cast<NodeCatalogue*>(n);
        connect(catToDel, SIGNAL(deleteFinished(NodeCatalogue*,bool)), this, SLOT(catalogueDeleteFinsihed(NodeCatalogue*,bool)));

        QThread* thread = new QThread;
        backgroundTask = new BackgroundTask( [=] { catToDel->removeFromDB(); } );
        backgroundTask->moveToThread(thread);

        connect(thread, SIGNAL(started()), backgroundTask, SLOT(go()));
        connect(backgroundTask, SIGNAL(finished()), this, SLOT(backgroundTaskFinished()));
        connect(backgroundTask, SIGNAL(finished()), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }
}

void MainWindow::on_actionDiskNew_triggered(bool updateMode)
{
    NodeCatalogue* cat = NULL;
    NodeDisk* disk = NULL;
    QModelIndex disk_usQmi;

    if (updateMode)
    {
        disk = getCurrentDisk(&disk_usQmi);
        Q_ASSERT(disk);
        Node* testCat = disk->getParent();
        if (testCat != Node::getRootNode()) cat = static_cast<NodeCatalogue*>(testCat);
    }
    else
    {
        QModelIndex current_usQmi = tms->mapToSource(ui->treeView->currentIndex());
        if (current_usQmi.isValid())
        {
            Node* walkUp = static_cast<Node*>(current_usQmi.internalPointer());

            while(true)
            {
                if (walkUp->getType() == TYPE_CAT)
                {
                    cat = static_cast<NodeCatalogue*>(walkUp);
                    break;
                }
                else if (walkUp->getType() == TYPE_DISK)
                {
                    if (walkUp->getParent() == Node::getRootNode()) break;
                    cat = static_cast<NodeCatalogue*>(walkUp->getParent());
                    break;
                }
                else
                {
                    walkUp = walkUp->getParent();
                }
            }
        }
    }


    DlgNewDisk* ndd = new DlgNewDisk(this, (cat == NULL ? 0 : cat->getID()));
    if (updateMode) ndd->updateMode(disk->getName(), disk->getCatPath());
    if (ndd->exec() != QDialog::Accepted) return;

    qint64 targetCatID = ndd->getSelectedCatalogue();
    QString newDiskName = ndd->getNewDiskName();
    QString newLocation = ndd->getScanLocation();

    Q_ASSERT(runningCataloguer == NULL);
    Q_ASSERT(progressDialog == NULL);

    progressDialog = new QProgressDialog("Cataloguing...", "Cancel", 0, 0, this);
    progressDialog->setWindowTitle("Cataloguing Progress");
    progressDialog->setMinimumWidth(300);
    progressDialog->setLabelText(QString("Initialising..."));
    progressDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);

    QThread* thread = new QThread;
    runningCataloguer = new Cataloguer(targetCatID, newDiskName, newLocation);
    runningCataloguer->moveToThread(thread);

    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cataloguerAbort()));
    connect(thread, SIGNAL(started()), runningCataloguer, SLOT(go()));
    connect(runningCataloguer, SIGNAL(finished(NodeDisk*)), this, SLOT(cataloguerFinished(NodeDisk*)));
    connect(runningCataloguer, SIGNAL(finished(NodeDisk*)), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(runningCataloguer, SIGNAL(numObjectsFound(qint64)), this, SLOT(updateCataloguerProgress(qint64)));
    connect(runningCataloguer, SIGNAL(reindexing()), this, SLOT(updateCataloguerReindexing()));

    if (updateMode)
    {
        tm->removeNode(disk_usQmi, [&]
        {
            Node* diskParent = disk->getParent();
            diskParent->removeChild(disk);
        });
        runningCataloguer->updateMode(disk);
    }

    thread->start();
}

void MainWindow::on_actionDiskRename_triggered()
{
    if (ui->tableView->hasFocus() && (fm->getMode() == TableModel::MODE_CAT))
    {
        ui->tableView->edit(ui->tableView->currentIndex());
        return;
    }

    QModelIndex usQmi;
    NodeDisk* disk = getCurrentDisk(&usQmi);
    if (!disk) return;
    QModelIndex sQmi = tms->mapFromSource(usQmi);
    ui->treeView->scrollTo(sQmi);
    ui->treeView->edit(sQmi);
}

void MainWindow::on_actionDiskMove_triggered()
{
    QModelIndex usQmi;
    NodeDisk* disk = getCurrentDisk(&usQmi);
    if (!disk) return;

    Node* parentCat = static_cast<Node*>(disk->getParent());
    qint64 parentCatID = -1;
    if (parentCat != NodeRoot::getRootNode()) parentCatID = parentCat->getID();

    DlgMoveDisk* mdd = new DlgMoveDisk(this, parentCatID);
    if (mdd->exec() != QDialog::Accepted) return;

    qint64 targetCatID = mdd->getSelectedCatalogue();

    // remove from model
    //   remove from nodes
    // disk->move
    // add to nodes
    // add to model
    // set current index

    tm->removeNode(usQmi, [&] {
        Node* diskParent = disk->getParent();
        diskParent->removeChild(disk);
    });
    disk->moveToCatalogue(targetCatID);
    addNewDiskToAll(disk);
}

void MainWindow::on_actionDiskUpdate_triggered()
{
    on_actionDiskNew_triggered(true);
}

void MainWindow::on_actionDiskDelete_triggered()
{
    NodeDisk* diskToDel = getCurrentDisk(NULL);
    if (!diskToDel) return;

    QMessageBox msgBox;
    msgBox.setText(QString("Are you sure you want to delete the disk: '") + diskToDel->getName() + "'?");
    msgBox.setInformativeText("This cannot be un-done.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    if (msgBox.exec() == QMessageBox::Ok)
    {
        progressDialog = new QProgressDialog("Deleting disk...", QString(), 0, 0, this);
        progressDialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setMinimumDuration(500);
        progressDialog->setValue(0);

        connect(diskToDel, SIGNAL(deleteFinished(NodeDisk*,bool)), this, SLOT(diskDeleteFinsihed(NodeDisk*,bool)));

        QThread* thread = new QThread;
        backgroundTask = new BackgroundTask( [=] { diskToDel->removeFromDB(); } );
        backgroundTask->moveToThread(thread);

        connect(thread, SIGNAL(started()), backgroundTask, SLOT(go()));
        connect(backgroundTask, SIGNAL(finished()), this, SLOT(backgroundTaskFinished()));
        connect(backgroundTask, SIGNAL(finished()), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }
}

void MainWindow::on_actionDiskOpen_triggered()
{
    NodeDisk* n = getCurrentDisk(NULL);
    n->osOpen();
}

void MainWindow::on_actionDiskProperties_triggered()
{
    NodeDisk* n = getCurrentDisk(NULL);
    if (!n) return;
    DlgDiskDirProperties ddp(this, n);
    ddp.exec();
    setActions();
}

void MainWindow::on_actionDiskMount_triggered()
{
    NodeDisk* disk = getCurrentDisk(NULL);
    if (!disk) return;

    if (disk->mount())
    {
        setActions();
        statusLabel.setText("Mount command returned success");
    }
    else
    {
        Utils::errorMessageBox(this, "Mount command failed");
    }
}

void MainWindow::on_actionDiskUnmount_triggered()
{
    NodeDisk* disk = getCurrentDisk(NULL);
    if (!disk) return;

    if (disk->unmount())
    {
        setActions();
        statusLabel.setText("Un-mount command returned success");
    }
    else
    {
        Utils::errorMessageBox(this, "Un-mount command failed");
    }
}

void MainWindow::on_actionDirGo_triggered()
{
    if (tableSelectedDir && dfSelectedDirAccessCheck()) dfDirGo(tableSelectedDir->getID());
}

void MainWindow::on_actionDirOpen_triggered()
{
    if (ui->treeView->hasFocus())
    {
        Q_ASSERT(treeSelectedDir);
        treeSelectedDir->osOpen();
    }
    else if (ui->tableView->hasFocus())
    {
        Q_ASSERT(tableSelectedDir);
        tableSelectedDir->osOpen();
    }
    else Q_ASSERT(false);
}

void MainWindow::on_actionDirProperties_triggered()
{
    if (ui->treeView->hasFocus())
    {
        Q_ASSERT(treeSelectedDir);
        DlgDirProperties pd(this, treeSelectedDir);
        pd.exec();
    }
    else if (ui->tableView->hasFocus())
    {
        Q_ASSERT(tableSelectedDir);
        DlgDirProperties pd(this, tableSelectedDir);
        pd.exec();
    }
    else Q_ASSERT(false);
}

void MainWindow::on_actionFileOpen_triggered()
{
    Q_ASSERT(tableSelectedFile);
    tableSelectedFile->osOpen();
}

void MainWindow::on_actionFileOpenContaining_triggered()
{
    if (tableSelectedFile) tableSelectedFile->osOpenContainer();
}

void MainWindow::on_actionFileProperties_triggered()
{
    Q_ASSERT(tableSelectedFile);
    DlgFileProperties pd(this, tableSelectedFile);
    pd.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    DlgAbout d;
    d.exec();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_buttonGoUp_clicked()
{
    if (ui->locSearch->getMode() == LocSearch::RESULTS)
    {
        returnFromSearch();
    }
    else
    {
        QModelIndex newQmi = ui->treeView->currentIndex().parent();
        if (!newQmi.isValid()) return;
        ui->treeView->setCurrentIndex(newQmi);
    }
}

void MainWindow::on_actionDelete_triggered()
{
    // Toolbar delete
    if (ui->tableView->hasFocus())
    {
        on_actionDiskDelete_triggered();
        return;
    }

    Node* t = getCurrentTreeItem();
    if (!t) return;
    if (t->getType() == TYPE_CAT) return on_actionCatalogueDelete_triggered();
    if (t->getType() == TYPE_DISK) return on_actionDiskDelete_triggered();
}

void MainWindow::on_actionRename_triggered()
{
    // Toolbar rename
    if (ui->treeView->hasFocus()) ui->treeView->edit(ui->treeView->currentIndex());
    else if (ui->tableView->hasFocus()) ui->tableView->edit(ui->tableView->currentIndex());
    else Q_ASSERT(false);
}


void MainWindow::on_actionCXONLYDiskGo_triggered()
{
    // Treeview is at catalogue
    // Disk double clicked in tableview
    // TreeView column 0 role UserRole is the int id of the dir

    QModelIndex dfSelected = fms->mapToSource(ui->tableView->currentIndex());
    qint64 diskID = fm->data(dfSelected, ROLE_ID).toLongLong();
    dfDirGo(diskID); // Actually this will work despite the comments in the function
}

void MainWindow::on_actionCxtSearchResultGo_triggered()
{
    navigateToSearchResult();
}

void MainWindow::on_actionCxtSearchResultOpen_triggered()
{
    QModelIndex qmi = ui->tableView->currentIndex();
    if (!qmi.isValid()) return;
    SearchResult* sr = static_cast<SearchResult*>(qmi.internalPointer());

    sr->osOpen();
}

void MainWindow::on_actionCxtSearchResultOpenContaining_triggered()
{
    QModelIndex qmi = ui->tableView->currentIndex();
    if (!qmi.isValid()) return;
    SearchResult* sr = static_cast<SearchResult*>(qmi.internalPointer());

    sr->osOpenContainer();
}

void MainWindow::on_actionCxtSearchResultProperties_triggered()
{
    QModelIndex qmi = ui->tableView->currentIndex();
    if (!qmi.isValid()) return;
    SearchResult* sr = static_cast<SearchResult*>(qmi.internalPointer());

    if (sr->getType() == TYPE_DISK)
    {
        DlgDiskDirProperties ddp(this, sr->getNodeDisk());
        ddp.exec();
        setActions();
    }
    else if (sr->getType() == TYPE_DIR)
    {
        DlgDirProperties pd(this, sr->getDDir());
        pd.exec();
    }
    else if (sr->getType() >= TYPE_FILE)
    {
        DlgFileProperties pd(this, sr->getDFile());
        pd.exec();
    }
}

void MainWindow::on_splitter_splitterMoved(int pos, int index)
{
    if (index != 1) return;
    ui->treeView->header()->setMinimumSectionSize(pos - 4); // There seems to be a small difference here to jump over
}

void MainWindow::on_tableView_activated(const QModelIndex &index)
{
    if (searchModel && (ui->tableView->model() == searchModel)) // search results double click
    {
        navigateToSearchResult();
    }
    else // normal mode
    {
        QModelIndex ufqmi = fms->mapToSource(index);
        qint64 targetID = fm->data(ufqmi, ROLE_ID).toLongLong();
        int targetType = fm->data(ufqmi, ROLE_TYPE).toInt();

        if (targetType == TYPE_DISK)
        {
            // Treeview is at the root or a catalogue
            // Search the treeview starting at its currently selected item
            // and search its children non-recursively for a disk

            QModelIndex firstDiskInCurrent = tms->index(0, 0, ui->treeView->currentIndex());
            QModelIndexList treeNewCSIList = tms->match(firstDiskInCurrent, ROLE_ID, targetID, -1, Qt::MatchExactly);
            Q_ASSERT(treeNewCSIList.count() == 1);
            QModelIndex newTarget = treeNewCSIList[0];

            ui->treeView->scrollTo(newTarget);
            ui->treeView->expand(newTarget);
            ui->treeView->setCurrentIndex(newTarget);

        }
        else if (targetType == TYPE_DIR)
        {
            if (dfSelectedDirAccessCheck()) dfDirGo(targetID);
        }
        else if (targetType >= TYPE_FILE)
        {
            on_actionFileProperties_triggered();
        }
    }
}

void MainWindow::on_treeView_clicked(const QModelIndex& /*index*/)
{
    // This function handles user clicking on already selected treeview item when in search mode
    // Table should return to dirs/files mode
    // clicked() is only called when the user clicks on a valid node
    // clicked() is called after treeview_current_changed
    // So, if we get to here and
    if (ui->locSearch->getMode() == LocSearch::RESULTS)
    {
        returnFromSearch();
    }
}

bool MainWindow::getConfigDatabase()
{
    QVariant qvDBFileName = settings.value("dbfile");
    if (qvDBFileName == QVariant()) return false;
    QString fileName = qvDBFileName.toString();
    return dbman.openDB(fileName);
}

void MainWindow::databaseJustOpened()
{
    Node::createRoot();

    // load tree
    tm = new TreeModel(this, Node::getRootNode());
    tms = new QSortFilterProxyModel();
    tms->setSourceModel(tm);
    tms->setSortCaseSensitivity(Qt::CaseInsensitive);
    QItemSelectionModel *m = ui->treeView->selectionModel();    // http://doc.qt.io/qt-5/qabstractitemview.html#setModel
    ui->treeView->setModel(tms);
    delete m;

    // Handle tree renaming items. Trigger updating the location text box
    connect(tm, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(treeView_dataChanged(QModelIndex,QModelIndex)));

    // This is the main response to user clicking a different tree node
    connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(treeView_current_changed(const QModelIndex&, const QModelIndex&)));

    // load table
    fm = new TableModel(NULL);
    fms = new TableSorter();
    fms->setSourceModel(fm);
    fms->setSortCaseSensitivity(Qt::CaseInsensitive);

    connect(fm, SIGNAL(requestRenameDisk(qint64,QString)), this, SLOT(requestRenameDisk(qint64,QString)));

    ui->actionDatabaseClose->setEnabled(true);
    ui->actionDatabaseProperties->setEnabled(true);
    ui->actionSearch->setEnabled(true);
    ui->actionCatalogueNew->setEnabled(true);
    ui->actionDiskNew->setEnabled(true);

    ui->treeView->expandToDepth(0);

    tableToFilesDirs();

    DBStats dbstats = dbman.getStats();
    QString allStats("Database opened. ");
    allStats += QLocale(QLocale::English).toString(dbstats.numCats) + " catalogues, ";
    allStats += QLocale(QLocale::English).toString(dbstats.numDisks) + " disks, ";
    allStats += QLocale(QLocale::English).toString(dbstats.numDirs) + " directories, ";
    allStats += QLocale(QLocale::English).toString(dbstats.numFiles) + " files. ";
    allStats += "Database size: " + fileSizeToHR(dbstats.size) + ".";
    statusLabel.setText(allStats);
    statusLabelHold = true;
    QTimer::singleShot(4000, [&] { statusLabelHold = false; } );
}

void MainWindow::backgroundTaskFinished()
{
    Q_ASSERT(backgroundTask);
    Q_ASSERT(progressDialog);
    delete backgroundTask;
    delete progressDialog;
    backgroundTask = NULL;
    progressDialog = NULL;
}

void MainWindow::updateCataloguerProgress(qint64 numObjects)
{
    progressDialog->setLabelText(QString("Cataloguing: %1 objects found...").arg(QLocale(QLocale::English).toString(numObjects)));
}

void MainWindow::updateCataloguerReindexing()
{
    progressDialog->setLabelText(QString("Re-indexing..."));
}

void MainWindow::cataloguerFinished(NodeDisk* newDisk)
{
    if (newDisk)
    {
        const QStringList& accessDeniedPaths = runningCataloguer->getAccessDeniedPaths();
        if (accessDeniedPaths.size() > 0)
        {
            DlgAccessDenieds d(this, accessDeniedPaths);
            d.exec();
        }

        addNewDiskToAll(newDisk);
    }

    int e = runningCataloguer->getError();
    if (e) Utils::errorMessageBoxNonBlocking(this, QString("Failed to catalogue the disk. Error = %1").arg(e));

    delete runningCataloguer;
    delete progressDialog;
    runningCataloguer = NULL;
    progressDialog = NULL;
}

void MainWindow::cataloguerAbort()
{
    // this will run in the same thread that cataloguerFinished would run in, so no race possible here
    if (runningCataloguer) runningCataloguer->abort();
}

void MainWindow::catalogueDeleteFinsihed(NodeCatalogue* catToDel, bool success)
{
    if (!success)
    {
        Utils::errorMessageBoxNonBlocking(this, "Error deleting catalogue from database");
        return;
    }

    QModelIndex catQmi = tm->getQmiForCatID(catToDel->getID());
    tm->removeNode(catQmi, [&] { Node::getRootNode()->removeChild(catToDel); } );
    delete catToDel;

    clearDataIfLast();
}

void MainWindow::diskDeleteFinsihed(NodeDisk* diskToDel, bool success)
{
    if (!success)
    {
        Utils::errorMessageBoxNonBlocking(this, "Error deleting disk from database");
        return;
    }

    // Get usQmi for Disk
    QModelIndex qmiCat = QModelIndex();
    if (diskToDel->getCatID()) qmiCat = tm->getQmiForCatID(diskToDel->getCatID());

    //qmiCat is now at root (invalid) or a catalogue

    QModelIndex diskQmi = QModelIndex();
    QModelIndexList matches = tm->match(tm->index(0, 0, qmiCat), ROLE_ID, diskToDel->getID(), -1, Qt::MatchExactly);
    foreach(QModelIndex matchQmi, matches)
    {
        Node* n = static_cast<Node*>(matchQmi.internalPointer());
        if (n->getType() == TYPE_DISK)
        {
            diskQmi = matchQmi;
            break;
        }
    }

    Q_ASSERT(diskQmi.isValid());

    // diskQmi is now at the disk to remove, or QModelIndex() in the case of logic error...

    tm->removeNode(diskQmi, [&] { diskToDel->getParent()->removeChild(diskToDel); } );

    // Finally, reload DF if this was a tableview delete
    if (fm->reloadCat())
    {
        if (tableSelectedFile) { delete tableSelectedFile; tableSelectedFile = NULL; }
        if (tableSelectedDir) { delete tableSelectedDir; tableSelectedDir = NULL; }
        if (tableSelectedDisk) { tableSelectedDisk = NULL; }
    }

    delete diskToDel;

    clearDataIfLast();
}

void MainWindow::clearDataIfLast()
{
    if (Node::getRootNode()->numChildren() == 0)
    {
        treeView_current_changed(QModelIndex(), QModelIndex()); // This function will do the right clear up
        fm->clear();
    }
}

void MainWindow::requestRenameDisk(qint64 diskID, QString newName)
{
    // This comes from the TableModel. The current index in the tree model must be the parent catalogue, so...
    QModelIndex usQmi = tms->mapToSource(ui->treeView->currentIndex());
    bool result = tm->renameDisk(usQmi, diskID, newName);
    if (result)
    {
        // Need to reload fm completely because it has no memory of its data apart from the DB
        if (fm->reloadCat())
        {
            if (tableSelectedFile) { delete tableSelectedFile; tableSelectedFile = NULL; }
            if (tableSelectedDir) { delete tableSelectedDir; tableSelectedDir = NULL; }
            if (tableSelectedDisk) { tableSelectedDisk = NULL; }
        }
    }
}

Node *MainWindow::getCurrentTreeItem() const
{
    QModelIndex sQmi = ui->treeView->currentIndex();
    if (sQmi == QModelIndex()) return NULL;
    return static_cast<Node*>(tms->mapToSource(sQmi).internalPointer());
}

NodeDisk* MainWindow::getCurrentDisk(QModelIndex* p_usQmi) const
{
    // Examine TreeView and TableView to get the "current" disk.
    // 1. If tableview has a tableSelectedDisk, that is the current disk, return it.
    // 2. If treeview is at a disk or any subdir of that disk, that disk is the current disk.

    if (p_usQmi) *p_usQmi = QModelIndex();

    QModelIndex sQmi = ui->treeView->currentIndex();
    if (!sQmi.isValid()) return NULL;
    QModelIndex usQmi = tms->mapToSource(sQmi);
    Node* node = static_cast<Node*>(usQmi.internalPointer());

    if (node->getType() == TYPE_CAT)
    {
        // Tree is at a cat
        if (!tableSelectedDisk) return NULL;

        QModelIndexList matches = tm->match(tm->index(0, 0, usQmi), ROLE_ID, tableSelectedDisk->getID(), -1, Qt::MatchExactly);
        Q_ASSERT(matches.size() == 1);

        if (p_usQmi) *p_usQmi = matches[0];
        return tableSelectedDisk;
    }
    else if (node->getType() == TYPE_DISK)
    {
        if (p_usQmi) *p_usQmi = usQmi;
        return static_cast<NodeDisk*>(node);
    }
    else if (node->getType() == TYPE_DIR)
    {
        while(true)
        {
            usQmi = usQmi.parent();
            Node* n = static_cast<Node*>(usQmi.internalPointer());
            if (n->getType() == TYPE_DISK)
            {
                if (p_usQmi) *p_usQmi = usQmi;
                return static_cast<NodeDisk*>(n);
            }
        }
    }
    return NULL;
}

void MainWindow::addNewDiskToAll(NodeDisk* disk)
{
    Node* addTo = Node::getRootNode()->catFromID(disk->getCatID());
    if (!addTo) addTo = Node::getRootNode();
    QModelIndex qmi = tm->addDisk(disk, [&] { addTo->addChild(disk); } );
    ui->treeView->setCurrentIndex(tms->mapFromSource(qmi));
}

void MainWindow::returnFromSearch()
{
    ui->locSearch->clearSearch();
    ui->buttonGoUp->setIcon(iconGoUp);
    tableToFilesDirs();
    delete searchModel;
    searchModel = NULL;
}

void MainWindow::handleLocSearchGotFocus()
{
    ui->buttonGoUp->setIcon(iconGoPrevious);
}

void MainWindow::handleLocSearchLostFocus()
{
    switch(ui->locSearch->getMode())
    {
    case LocSearch::RESULTS:
        return;
    case LocSearch::EDITING:
        if (ui->tableView->model() == fms)
        {
            ui->buttonGoUp->setIcon(iconGoUp);
            ui->locSearch->toLocationMode();

        }
        else // showing search results
        {
            ui->locSearch->toResultsMode();
        }
    }
}

void MainWindow::handleLocSearchEsc()
{
    ui->tableView->setFocus(); // set the focus elsewhere, allow focusout to handle everything else
}

void MainWindow::handleDoSearch(QString text)
{
    SearchModel* oldSearchModel = NULL;

    if (ui->tableView->model() == searchModel) // already displaying previous search results
    {
        // About to delete the model so rescue the column widths here
        settings.setValue("scolwidth0", ui->tableView->columnWidth(0) < 100 ? 100 : ui->tableView->columnWidth(0));
        settings.setValue("scolwidth1", ui->tableView->columnWidth(1) < 100 ? 100 : ui->tableView->columnWidth(1));
        oldSearchModel = searchModel;
    }
    else
    {
        settings.setValue("dfcolwidth0", ui->tableView->columnWidth(0) < 50 ? 50 : ui->tableView->columnWidth(0));
        settings.setValue("dfcolwidth1", ui->tableView->columnWidth(1) < 50 ? 50 : ui->tableView->columnWidth(1));
        settings.setValue("dfcolwidth2", ui->tableView->columnWidth(2) < 50 ? 50 : ui->tableView->columnWidth(2));
        settings.setValue("dfcolwidth3", ui->tableView->columnWidth(3) < 50 ? 50 : ui->tableView->columnWidth(3));
        settings.setValue("dfcolwidth4", ui->tableView->columnWidth(4) < 50 ? 50 : ui->tableView->columnWidth(4));
        settings.setValue("dfcolwidth5", ui->tableView->columnWidth(5) < 50 ? 50 : ui->tableView->columnWidth(5));
    }

    ui->tableView->setFocus();

    searchModel = new SearchModel(this);
    searchModel->search(text);

    QItemSelectionModel *m = ui->tableView->selectionModel();    // http://doc.qt.io/qt-5/qabstractitemview.html#setModel
    ui->tableView->setModel(searchModel);
    delete m;

    if (oldSearchModel) delete oldSearchModel;

    connect(ui->tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(tableView_s_current_changed(QModelIndex,QModelIndex)));
    ui->tableView->setColumnWidth(0, settings.value("scolwidth0", 250).toInt());
    ui->tableView->setColumnWidth(1, settings.value("scolwidth1", 440).toInt());
    ui->tableView->setSortingEnabled(false);

    qint64 numCats = searchModel->getNumCats();
    qint64 numDisks = searchModel->getNumDisks();
    qint64 numDirs = searchModel->getNumDirs();
    qint64 numFiles = searchModel->getNumFiles();
    qint64 numAll = numCats + numDisks + numDirs + numFiles;
    statusLabel.setText(QString::number(numAll) + " results. (" + QString::number(numCats) + " catalogues, " +
                        QString::number(numDisks) + " disks, " + QString::number(numDirs) + " directories and " +
                        QString::number(numFiles) + " files).");
}

void MainWindow::navigateToSearchResult()
{
    SearchResult* sr = static_cast<SearchResult*>(ui->tableView->currentIndex().internalPointer());
    QList<QPair<qint64,qint64>> fullIDLocation = sr->getFullIDLocation();
    // Now need to walk down the tree ensuring each level is expanded
    returnFromSearch();

    QModelIndex qmi = QModelIndex();
    QModelIndex sqmi;
    qint64 highlightFileID = -1;
    auto it = fullIDLocation.begin();

    // Handle first pair, the catalogue
    if ((*it).second != 0) // there is a catalogue
    {
        QModelIndexList matches = tm->match(tm->index(0, 0, qmi), ROLE_ID, (*it).second, -1, Qt::MatchExactly);
        // Found: catalogue (and possibly disk) with that ID
        foreach(qmi, matches)
        {
            if (tm->data(qmi, ROLE_TYPE).toInt() == TYPE_CAT)
            {
                sqmi = tms->mapFromSource(qmi);
                ui->treeView->scrollTo(sqmi);
                ui->treeView->expand(sqmi);
                break;
            }
        }
    }
    it++;

    if (it != fullIDLocation.end())
    {
        // Handle second pair, the disk
        QModelIndexList matches = tm->match(tm->index(0, 0, qmi), ROLE_ID, (*it).second, -1, Qt::MatchExactly);
        // Found: disk (and possibly catalogue) with that ID
        foreach(qmi, matches)
        {
            if (tm->data(qmi, ROLE_TYPE).toInt() == TYPE_DISK)
            {
                sqmi = tms->mapFromSource(qmi);
                ui->treeView->scrollTo(sqmi);
                ui->treeView->expand(sqmi);
                break;
            }
        }
        it++; // Advance from disk to rootdir

        if (it != fullIDLocation.end())
        {
            it++; // Advance over rootdir - we don't need it

            // Now handle the rest. 0-n dirs, then 0-1 file.

            for (; it != fullIDLocation.end(); it++)
            {
                if ((*it).first >= TYPE_FILE) { highlightFileID = (*it).second; break; } // Got to a file, break

                QModelIndexList matches2 = tm->match(tm->index(0, 0, qmi), ROLE_ID, (*it).second, -1, Qt::MatchExactly);
                Q_ASSERT(matches2.size() == 1);
                qmi = matches2[0];
                sqmi = tms->mapFromSource(qmi);
                ui->treeView->scrollTo(sqmi);
                ui->treeView->expand(sqmi);
            }
        }
    }
    ui->treeView->setCurrentIndex(sqmi);

    if (highlightFileID != -1) // Highlight the found file
    {
        QModelIndexList matches = fms->match(fms->index(0, 0), ROLE_ID, highlightFileID, -1, Qt::MatchExactly);
        foreach(QModelIndex found, matches)
        {
            QVariant qv = fms->data(found, ROLE_TYPE);
            if (qv.toInt() >= TYPE_FILE)
            {
                ui->tableView->setCurrentIndex(found);
                QCoreApplication::processEvents();
                ui->tableView->scrollTo(found);
            }
        }
    }
}

void MainWindow::treeView_collapsed(const QModelIndex& sQmi)
{
    // 1st, set table to display just collapsed node
    // 2nd, only do this if current was indeed below just collapsed node

    QModelIndex current = ui->treeView->currentIndex();
    if (sQmi == current) return;

    // Is current a child of sQmi? Only reset the display if it is.
    QModelIndex check = current.parent();
    while(check.isValid())
    {
        if (check == sQmi) return ui->treeView->setCurrentIndex(sQmi);
        check = check.parent();
    }
}

void MainWindow::treeView_dataChanged(const QModelIndex& /*start*/, const QModelIndex& /*end*/)
{
    setLocationText();
}

void MainWindow::treeView_current_changed(const QModelIndex& /*current*/, const QModelIndex& previous)
{
    if (tableSelectedFile) { delete tableSelectedFile; tableSelectedFile = NULL; }
    if (tableSelectedDir) { delete tableSelectedDir; tableSelectedDir = NULL; }
    if (treeSelectedDir) { delete treeSelectedDir; treeSelectedDir = NULL; }
    if (tableSelectedDisk) { tableSelectedDisk = NULL; }

    Node* t = getCurrentTreeItem();
    if (!t)
    {
        ui->locSearch->setLocationText();
        statusLabel.setText("Create a new catalogue or disk to continue.");
        setActions();
        return;
    }

    if (ui->locSearch->getMode() == LocSearch::RESULTS)
    {
        returnFromSearch();
    }

    if (t->getType() == TYPE_CAT)
    {
        fm->loadCat(t->getID());
        if (!statusLabelHold) statusLabel.setText(t->summaryText());
        setActions();
    }
    else if (t->getType() == TYPE_DISK)
    {
        fm->loadDir((static_cast<NodeDisk*>(t))->getRootDirID());
        statusLabel.setText(t->summaryText());
        setActions();
    }
    else if (t->getType() == TYPE_DIR)
    {
        NodeDir* nd = static_cast<NodeDir*>(t);
        if (nd->isAccessDenied())
        {
            Utils::errorMessageBox(this, "Access was denied.");
            ui->treeView->setCurrentIndex(previous);
            return;
        }

        fm->loadDir(t->getID());
        statusLabel.setText(t->summaryText());

        treeSelectedDir = new DDir(t->getID());
        if (!treeSelectedDir->loadFromDB()) Utils::errorMessageBox(this, "Database error");
        treeSelectedDir->loadFromFileSystem();

        setActions();
    }
    else
    {
        Q_ASSERT(false);
    }

    setLocationText();
}

void MainWindow::treeViewContextMenu(QPoint qp)
{
    QModelIndex sQmi = ui->treeView->indexAt(qp);
    if (!sQmi.isValid()) return;
    QModelIndex usQmi = tms->mapToSource(sQmi);
    Node* currentTreeNode = static_cast<Node*>(usQmi.internalPointer());

    QMenu contextMenu;

    if (currentTreeNode->getType() == TYPE_CAT)
    {
        contextMenu.addAction(ui->actionCatalogueRename);
        contextMenu.addAction(ui->actionCatalogueDelete);
        contextMenu.addAction(ui->actionDiskNew);
    }
    else if (currentTreeNode->getType() == TYPE_DISK)
    {
        contextMenu.addAction(ui->actionDiskRename);
        contextMenu.addAction(ui->actionDiskMove);
        contextMenu.addAction(ui->actionDiskUpdate);
        contextMenu.addAction(ui->actionDiskDelete);
        contextMenu.addAction(ui->actionDiskOpen);
        contextMenu.addAction(ui->actionDiskProperties);
        contextMenu.addSeparator();
        contextMenu.addAction(ui->actionDiskMount);
        contextMenu.addAction(ui->actionDiskUnmount);
    }
    else if (currentTreeNode->getType() == TYPE_DIR)
    {
        contextMenu.addAction(ui->actionDirOpen);
        contextMenu.addAction(ui->actionDirProperties);
    }
    else
    {
        return;
    }

    contextMenu.exec(ui->treeView->mapToGlobal(qp + QPoint(2,2))); // or non block popup
}

void MainWindow::tableViewContextMenu(QPoint qp)
{
    QMenu contextMenu;
    bool doMenu = false;

    if (searchModel && (ui->tableView->model() == searchModel)) // search results
    {
        QModelIndex qmi = ui->tableView->currentIndex();
        if (qmi.isValid())
        {
            contextMenu.addAction(ui->actionCxtSearchResultGo);
            contextMenu.addAction(ui->actionCxtSearchResultOpen);
            contextMenu.addAction(ui->actionCxtSearchResultOpenContaining);
            contextMenu.addAction(ui->actionCxtSearchResultProperties);
            doMenu = true;
        }
    }
    else
    {
        if (tableSelectedFile)
        {
            contextMenu.addAction(ui->actionFileOpen);
            contextMenu.addAction(ui->actionFileOpenContaining);
            contextMenu.addAction(ui->actionFileProperties);

            doMenu = true;
        }
        else if (tableSelectedDir)
        {
            contextMenu.addAction(ui->actionDirGo);
            contextMenu.addAction(ui->actionDirOpen);
            contextMenu.addAction(ui->actionDirProperties);

            doMenu = true;
        }
        else if (tableSelectedDisk)
        {
            contextMenu.addAction(ui->actionCXONLYDiskGo);
            contextMenu.addAction(ui->actionDiskRename);
            contextMenu.addAction(ui->actionDiskMove);
            contextMenu.addAction(ui->actionDiskUpdate);
            contextMenu.addAction(ui->actionDiskDelete);
            contextMenu.addAction(ui->actionDiskOpen);
            contextMenu.addAction(ui->actionDiskProperties);
            contextMenu.addSeparator();
            contextMenu.addAction(ui->actionDiskMount);
            contextMenu.addAction(ui->actionDiskUnmount);

            doMenu = true;
        }
        else
        {
            contextMenu.addAction(ui->actionDiskNew);
            doMenu = true;
        }
    }

    if (doMenu)
    {
        QPoint qp2 = ui->tableView->mapToGlobal(qp);
        qp2.setY(qp2.y() + ui->tableView->horizontalHeader()->height());
        contextMenu.exec(qp2 + QPoint(2,2)); // or non block popup
    }
}

void MainWindow::tableToFilesDirs()
{
    if (ui->tableView->model())
    {
        Q_ASSERT(ui->tableView->columnWidth(0) != 0);
        Q_ASSERT(ui->tableView->columnWidth(1) != 0);
        settings.setValue("scolwidth0", ui->tableView->columnWidth(0) < 100 ? 100 : ui->tableView->columnWidth(0));
        settings.setValue("scolwidth1", ui->tableView->columnWidth(1) < 100 ? 100 : ui->tableView->columnWidth(1));
    }

    QItemSelectionModel *m = ui->tableView->selectionModel();    // http://doc.qt.io/qt-5/qabstractitemview.html#setModel
    ui->tableView->setModel(fms);
    delete m;

    connect(ui->tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(tableView_ddf_current_changed(QModelIndex,QModelIndex)));

    ui->tableView->setColumnWidth(0, settings.value("dfcolwidth0", 300).toInt());
    ui->tableView->setColumnWidth(1, settings.value("dfcolwidth1", 80).toInt());
    ui->tableView->setColumnWidth(2, settings.value("dfcolwidth2", 100).toInt());
    ui->tableView->setColumnWidth(3, settings.value("dfcolwidth3", 65).toInt());
    ui->tableView->setColumnWidth(4, settings.value("dfcolwidth4", 65).toInt());
    ui->tableView->setColumnWidth(5, settings.value("dfcolwidth5", 100).toInt());
    ui->tableView->setSortingEnabled(true);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);
}

bool MainWindow::dfSelectedDirAccessCheck()
{
    Q_ASSERT(tableSelectedDir);

    if (tableSelectedDir->isAccessDenied())
    {
        Utils::errorMessageBox(this, "Access was denied.");
        return false;
    }
    return true;
}

void MainWindow::dfDirGo(qint64 targetID)
{
    // Treeview is at disk or dir
    // Dir double clicked in tableview is a child dir of
    // dir at current
    // TreeView column 0 role UserRole is the int id of the dir

    QModelIndex firstChild = tms->index(0, 0, ui->treeView->currentIndex());
    QModelIndexList treeNewCSIList = tms->match(firstChild, ROLE_ID, targetID, 1, Qt::MatchExactly);
    Q_ASSERT(treeNewCSIList.count() == 1);
    QModelIndex newTarget = treeNewCSIList[0];
    ui->treeView->scrollTo(newTarget);
    ui->treeView->expand(newTarget);
    ui->treeView->setCurrentIndex(newTarget);
}

void MainWindow::setLocationText()
{
    QList<QString> pathList;
    Node* cti = getCurrentTreeItem();
    while(cti != Node::getRootNode())
    {
        if (cti->getType() == TYPE_DIR)
        {
            pathList.append(cti->getName());
            pathList.append("/");
        }
        else if (cti->getType() == TYPE_DISK)
        {
            pathList.append(":");
            pathList.append(cti->getName());
        }
        else if (cti->getType() == TYPE_CAT)
        {
            pathList.append("/");
            pathList.append(cti->getName());
        }

        cti = cti->getParent();
    }

    qint64 finalLength = 0;
    foreach(QString t, pathList) finalLength += t.size();
    QString locSearchText;
    locSearchText.reserve(finalLength);
    while(pathList.size()) locSearchText += pathList.takeLast();
    ui->locSearch->setLocationText(locSearchText);
}

void MainWindow::focusChanged(QWidget* /*old*/, QWidget* /*now*/)
{
    setActions();
}

void MainWindow::tableView_ddf_current_changed(const QModelIndex& /*current*/, const QModelIndex &)
{
    if (tableSelectedFile) { delete tableSelectedFile; tableSelectedFile = NULL; }
    if (tableSelectedDir) { delete tableSelectedDir; tableSelectedDir = NULL; }
    if (tableSelectedDisk) { tableSelectedDisk = NULL; }

    QModelIndex tableCurrent = ui->tableView->currentIndex();
    if (!tableCurrent.isValid()) return;
    QModelIndex usQmi = fms->mapToSource(tableCurrent);

    qint64 id = fm->data(usQmi, ROLE_ID).toLongLong();
    int type = fm->data(usQmi, ROLE_TYPE).toInt();

    if (type >= TYPE_FILE)
    {
        tableSelectedFile = new DFile(id);
        if (!tableSelectedFile->loadFromDB()) Utils::errorMessageBox(this, "Database error");
        tableSelectedFile->loadFromFileSystem();
    }
    else if (type == TYPE_DIR)
    {
        tableSelectedDir = new DDir(id);
        if (!tableSelectedDir->loadFromDB()) Utils::errorMessageBox(this, "Database error");
        tableSelectedDir->loadFromFileSystem();
    }
    else if (type == TYPE_DISK)
    {
        tableSelectedDisk = Node::getRootNode()->diskFromIDRecursive(id);
    }

    setActions();
}

void MainWindow::tableView_s_current_changed(const QModelIndex& current, const QModelIndex &)
{
    SearchResult* sr = static_cast<SearchResult*>(current.internalPointer());
    sr->loadDObject();
    ui->actionCxtSearchResultOpen->setEnabled(sr->isReachable());
    ui->actionCxtSearchResultOpenContaining->setEnabled(sr->isContainerReachable());
}

void MainWindow::setActions()
{
    int selectSource = 0;
    int typeSelected = 0;
//    qint64 idSelected = 0;

    if (ui->treeView->hasFocus())
    {
        selectSource = 1;
        Node* n = getCurrentTreeItem();
        if (n)
        {
            typeSelected = n->getType();
//            idSelected = n->getID();
        }
    }
    else if (ui->tableView->hasFocus())
    {
        selectSource = 2;

        if (ui->tableView->model() == searchModel) // already displaying previous search results
        {
            // qDebug() << "setActions when search";
        }
        else
        {

            QModelIndex usQmi = fms->mapToSource(ui->tableView->currentIndex());
            if (usQmi.isValid())
            {
                typeSelected = fm->data(usQmi, ROLE_TYPE).toInt();
    //            idSelected = fm->data(usQmi, ROLE_ID).toLongLong();
            }
        }
    }

    if (typeSelected == TYPE_INVALID) // Nothing selected, disable everything
    {
        ui->actionCatalogueRename->setEnabled(false);
        ui->actionCatalogueDelete->setEnabled(false);

        ui->actionDiskRename->setEnabled(false);
        ui->actionDiskMove->setEnabled(false);
        ui->actionDiskUpdate->setEnabled(false);
        ui->actionDiskDelete->setEnabled(false);
        ui->actionDiskOpen->setEnabled(false);
        ui->actionDiskProperties->setEnabled(false);
        ui->actionDiskMount->setEnabled(false);
        ui->actionDiskUnmount->setEnabled(false);
        ui->actionCXONLYDiskGo->setEnabled(false);

        ui->actionDirGo->setEnabled(false);
        ui->actionDirOpen->setEnabled(false);
        ui->actionDirProperties->setEnabled(false);

        ui->actionFileOpen->setEnabled(false);
        ui->actionFileOpenContaining->setEnabled(false);
        ui->actionFileProperties->setEnabled(false);

        ui->actionRename->setEnabled(false);
        ui->actionDelete->setEnabled(false);
    }
    else if (typeSelected == TYPE_CAT)
    {
        ui->actionCatalogueRename->setEnabled(true);
        ui->actionCatalogueDelete->setEnabled(true);

        if (getCurrentDisk(NULL))
        {
            ui->actionDiskRename->setEnabled(false);
            ui->actionDiskMove->setEnabled(allowDiskMove());
            ui->actionDiskUpdate->setEnabled(true);
            ui->actionDiskDelete->setEnabled(false);
            ui->actionDiskOpen->setEnabled(allowDiskOpen());
            ui->actionDiskProperties->setEnabled(true);
            ui->actionDiskMount->setEnabled(allowDiskMount());
            ui->actionDiskUnmount->setEnabled(allowDiskUnmount());
            ui->actionCXONLYDiskGo->setEnabled(true);
        }
        else
        {
            ui->actionDiskRename->setEnabled(false);
            ui->actionDiskMove->setEnabled(false);
            ui->actionDiskUpdate->setEnabled(false);
            ui->actionDiskDelete->setEnabled(false);
            ui->actionDiskOpen->setEnabled(false);
            ui->actionDiskProperties->setEnabled(false);
            ui->actionDiskMount->setEnabled(false);
            ui->actionDiskUnmount->setEnabled(false);
            ui->actionCXONLYDiskGo->setEnabled(false);
        }

        ui->actionDirGo->setEnabled(false);
        ui->actionDirOpen->setEnabled(false);
        ui->actionDirProperties->setEnabled(false);

        ui->actionFileOpen->setEnabled(false);
        ui->actionFileOpenContaining->setEnabled(false);
        ui->actionFileProperties->setEnabled(false);

        ui->actionRename->setEnabled(true);
        ui->actionDelete->setEnabled(true);
    }
    else if (typeSelected == TYPE_DISK)
    {
        ui->actionCatalogueRename->setEnabled(false);
        ui->actionCatalogueDelete->setEnabled(false);

        ui->actionDiskRename->setEnabled(true);
        ui->actionDiskMove->setEnabled(allowDiskMove());
        ui->actionDiskUpdate->setEnabled(true);
        ui->actionDiskDelete->setEnabled(true);
        ui->actionDiskOpen->setEnabled(allowDiskOpen());
        ui->actionDiskProperties->setEnabled(true);
        ui->actionDiskMount->setEnabled(allowDiskMount());
        ui->actionDiskUnmount->setEnabled(allowDiskUnmount());
        ui->actionCXONLYDiskGo->setEnabled(true);

        ui->actionDirGo->setEnabled(false);
        ui->actionDirOpen->setEnabled(false);
        ui->actionDirProperties->setEnabled(false);

        ui->actionFileOpen->setEnabled(false);
        ui->actionFileOpenContaining->setEnabled(false);
        ui->actionFileProperties->setEnabled(false);

        ui->actionRename->setEnabled(true);
        ui->actionDelete->setEnabled(true);
    }
    else if (typeSelected == TYPE_DIR)
    {
        ui->actionCatalogueRename->setEnabled(false);
        ui->actionCatalogueDelete->setEnabled(false);

        ui->actionDiskRename->setEnabled(true);
        ui->actionDiskMove->setEnabled(allowDiskMove());
        ui->actionDiskUpdate->setEnabled(true);
        ui->actionDiskDelete->setEnabled(true);
        ui->actionDiskOpen->setEnabled(allowDiskOpen());
        ui->actionDiskProperties->setEnabled(true);
        ui->actionDiskMount->setEnabled(allowDiskMount());
        ui->actionDiskUnmount->setEnabled(allowDiskUnmount());
        ui->actionCXONLYDiskGo->setEnabled(false);

        ui->actionDirGo->setEnabled(true);
        ui->actionDirOpen->setEnabled(allowDirOpen(selectSource));
        ui->actionDirProperties->setEnabled(true);

        ui->actionFileOpen->setEnabled(false);
        ui->actionFileOpenContaining->setEnabled(false);
        ui->actionFileProperties->setEnabled(false);

        ui->actionRename->setEnabled(false);
        ui->actionDelete->setEnabled(false);
    }
    else if (typeSelected == TYPE_FILE)
    {
        ui->actionCatalogueRename->setEnabled(false);
        ui->actionCatalogueDelete->setEnabled(false);

        ui->actionDiskRename->setEnabled(true);
        ui->actionDiskMove->setEnabled(allowDiskMove());
        ui->actionDiskUpdate->setEnabled(true);
        ui->actionDiskDelete->setEnabled(true);
        ui->actionDiskOpen->setEnabled(allowDiskOpen());
        ui->actionDiskProperties->setEnabled(true);
        ui->actionDiskMount->setEnabled(allowDiskMount());
        ui->actionDiskUnmount->setEnabled(allowDiskUnmount());
        ui->actionCXONLYDiskGo->setEnabled(false);

        ui->actionDirGo->setEnabled(false);
        ui->actionDirOpen->setEnabled(false);
        ui->actionDirProperties->setEnabled(false);

        ui->actionFileOpen->setEnabled(allowFileOpen());
        ui->actionFileOpenContaining->setEnabled(allowFileOpenContaining());
        ui->actionFileProperties->setEnabled(true);

        ui->actionRename->setEnabled(false);
        ui->actionDelete->setEnabled(false);
    }
    else if (typeSelected == TYPE_OTHERFILEUNKNOWN)
    {
        ui->actionCatalogueRename->setEnabled(false);
        ui->actionCatalogueDelete->setEnabled(false);

        ui->actionDiskRename->setEnabled(true);
        ui->actionDiskMove->setEnabled(allowDiskMove());
        ui->actionDiskUpdate->setEnabled(true);
        ui->actionDiskDelete->setEnabled(true);
        ui->actionDiskOpen->setEnabled(allowDiskOpen());
        ui->actionDiskProperties->setEnabled(true);
        ui->actionDiskMount->setEnabled(allowDiskMount());
        ui->actionDiskUnmount->setEnabled(allowDiskUnmount());
        ui->actionCXONLYDiskGo->setEnabled(false);

        ui->actionDirGo->setEnabled(false);
        ui->actionDirOpen->setEnabled(false);
        ui->actionDirProperties->setEnabled(false);

        ui->actionFileOpen->setEnabled(false);
        ui->actionFileOpenContaining->setEnabled(allowFileOpenContaining());
        ui->actionFileProperties->setEnabled(true);

        ui->actionRename->setEnabled(false);
        ui->actionDelete->setEnabled(false);
    }
}

bool MainWindow::allowDiskMove()
{
    return (Node::getRootNode()->numCatalogues() > 0);
}

bool MainWindow::allowDiskMount()
{
    NodeDisk* disk = getCurrentDisk(NULL);
    if (!disk) return false;
    if (disk->hasMountCommand() && disk->catPathLocationEmpty())
        return true;
    return false;
}

bool MainWindow::allowDiskUnmount()
{
    NodeDisk* disk = getCurrentDisk(NULL);
    if (!disk) return false;
    if (disk->hasUnmountCommand() && disk->catPathLocationFull()) return true;
    return false;
}

bool MainWindow::allowDiskOpen()
{
    NodeDisk* dn = getCurrentDisk(NULL);
    if (dn) return dn->isReachable();
    return false;
}

bool MainWindow::allowDirOpen(int uiSource)
{
    if (uiSource == 1) return treeSelectedDir->isReachable();
    if (uiSource == 2) return tableSelectedDir->isReachable();
    return false;
}

bool MainWindow::allowFileOpen()
{
    return tableSelectedFile->isReachable();
}

bool MainWindow::allowFileOpenContaining()
{
     return tableSelectedFile->isContainerReachable();
}
