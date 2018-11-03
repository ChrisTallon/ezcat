#include <QMessageBox>

#include "utils.h"

void Utils::errorMessageBox(QWidget* parent, const QString& message)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle("Error");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

void Utils::errorMessageBoxNonBlocking(QWidget *parent, const QString &message)
{
    QMessageBox* msgBox = new QMessageBox(parent);
    msgBox->setWindowTitle("Error");
    msgBox->setText(message);
    msgBox->setIcon(QMessageBox::Critical);
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // Made on heap and this attr set, otherwise...
    msgBox->show(); // exec here would hang the event loop
}
