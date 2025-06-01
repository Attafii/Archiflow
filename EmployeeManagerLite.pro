
QT += core gui sql network networkauth
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = EmployeeManagerLite
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp 
	   

HEADERS += mainwindow.h

RESOURCES += \
    resources.qrc
