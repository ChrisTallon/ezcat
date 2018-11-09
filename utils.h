#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
public:
    static void errorMessageBox(const QString& message);
    static void errorMessageBoxNonBlocking(const QString& message);
};

#endif // UTILS_H
