QT       += core gui network positioning
QT += serialport
QT += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YourProjectName
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    Arduino.cpp

HEADERS += mainwindow.h \
    Arduino.h

FORMS += mainwindow.ui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = GestionDesProjets
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/projet.cpp \
    src/projetmanager.cpp \
    src/projetdialog.cpp \
    src/maplocation.cpp \
    src/durationestimator.cpp \
    src/searchfilter.cpp

HEADERS += \
    include/mainwindow.h \
    include/projet.h \
    include/projetmanager.h \
    include/projetdialog.h \
    include/maplocation.h \
    include/durationestimator.h \
    include/searchfilter.h

FORMS += \
    forms/mainwindow.ui \
    forms/projetdialog.ui \
    forms/maplocation.ui \
    forms/searchfilter.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += include/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
