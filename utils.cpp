#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"

#include "utils.h"

void Utils::errorMessageBox(const QString& message)
{
    QMessageBox msgBox(MainWindow::msgboxParent());
    msgBox.setWindowTitle("Error");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void Utils::errorMessageBoxNonBlocking(const QString &message)
{
    QMessageBox* msgBox = new QMessageBox(MainWindow::msgboxParent());
    msgBox->setWindowTitle("Error");
    msgBox->setText(message);
    msgBox->setIcon(QMessageBox::Critical);
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // Made on heap and this attr set, otherwise...
    msgBox->show(); // exec here would hang the event loop
}
