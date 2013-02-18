#-------------------------------------------------
#
# Project created by QtCreator 2013-01-27T13:05:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ComBomb
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    childform.cpp \
    cbtextedit.cpp \
    TargetIntf.cpp \
    opendialog.cpp \
    vt100.cpp

HEADERS  += mainwindow.h \
    childform.h \
    cbtextedit.h \
    TargetIntf.h \
    opendialog.h \
    vt100.h

FORMS    += mainwindow.ui \
    childform.ui \
    opendialog.ui

RESOURCES += \
    ComBomb.qrc

QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501
