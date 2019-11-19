

QT  += core gui charts widgets printsupport
Qt  += charts
QT  += datavisualization
requires(qtConfig(combobox))

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Infovizanalytics
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../../../include
LIBS += -L$$OUT_PWD/../../../lib

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        axisspecdata.cpp \
        axisspecdialog.cpp \
        colortable.cpp \
        datamanager.cpp \
        dataviewdialog.cpp \
        main.cpp \
        mainwindow.cpp \
        plotwindow.cpp \
        qcustomplot.cpp \
        surfacePlot.cpp \
        threedview.cpp \
        varseldialog.cpp

HEADERS += \
        axisspecdata.h \
        axisspecdialog.h \
        colortable.h \
        datamanager.h \
        dataviewdialog.h \
        mainwindow.h \
        plotwindow.h \
        qcustomplot.h \
        surfacePlot.h \
        threedview.h \
        varseldialog.h

FORMS += \
        axisspecdialog.ui \
        dataviewdialog.ui \
        mainwindow.ui \
        plotwindow.ui \
        varseldialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
