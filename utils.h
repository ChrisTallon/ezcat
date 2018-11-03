#ifndef UTILS_H
#define UTILS_H

#include <QString>

class QWidget;

class Utils
{
public:
    static void errorMessageBox(QWidget* parent, const QString& message);
    static void errorMessageBoxNonBlocking(QWidget* parent, const QString& message);
};

#endif // UTILS_H
