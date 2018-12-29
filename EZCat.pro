#-------------------------------------------------
#
# Project created by QtCreator 2018-09-04T20:42:05
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ezcat
TEMPLATE = app

CONFIG += c++14

QMAKE_CXXFLAGS += -Werror=return-type -Wextra -Wshadow -Wmissing-declarations -Winit-self -Woverloaded-virtual -Wold-style-cast
# -Wconversion

LIBS += -lblkid

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050900

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    treemodel.cpp \
    globals.cpp \
    cataloguer.cpp \
    locsearch.cpp \
    searchmodel.cpp \
    searchresult.cpp \
    backgroundtask.cpp \
    node.cpp \
    dirpropertieswidget.cpp \
    dfile.cpp \
    ddir.cpp \
    noderoot.cpp \
    nodedir.cpp \
    nodedisk.cpp \
    tablesorter.cpp \
    db.cpp \
    nodecatalogue.cpp \
    tablemodel.cpp \
    dlgnewdisk.cpp \
    dlgfileproperties.cpp \
    dlgdiskdirproperties.cpp \
    dlgmovedisk.cpp \
    dlgdbinfo.cpp \
    dlgdirproperties.cpp \
    dlgabout.cpp \
    dlgaccessdenieds.cpp \
    utils.cpp

HEADERS += \
        mainwindow.h \
    treemodel.h \
    globals.h \
    cataloguer.h \
    locsearch.h \
    searchmodel.h \
    searchresult.h \
    backgroundtask.h \
    node.h \
    dirpropertieswidget.h \
    dfile.h \
    ddir.h \
    noderoot.h \
    nodedir.h \
    nodedisk.h \
    tablesorter.h \
    db.h \
    nodecatalogue.h \
    tablemodel.h \
    dlgnewdisk.h \
    dlgfileproperties.h \
    dlgdiskdirproperties.h \
    dlgmovedisk.h \
    dlgdbinfo.h \
    dlgdirproperties.h \
    dlgabout.h \
    dlgaccessdenieds.h \
    utils.h

FORMS += \
        mainwindow.ui \
    newdiskdialog.ui \
    movediskdialog.ui \
    filepropertiesdialog.ui \
    diskdirproperties.ui \
    dirpropertieswidget.ui \
    dlgdbinfo.ui \
    dlgdirproperties.ui \
    dlgabout.ui \
    dlgaccessdenieds.ui

DISTFILES += \
    info.txt \
    db-schema.txt \
    LICENCE.txt \
    ezcat.desktop \
    README.md

target.path = /usr/bin
INSTALLS += target

menuentry.path = /usr/share/applications
menuentry.files = ezcat.desktop
INSTALLS += menuentry
