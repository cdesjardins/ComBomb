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
    vt100.h \
    LineFont.h

FORMS    += mainwindow.ui \
    childform.ui \
    opendialog.ui

RESOURCES += \
    ComBomb.qrc

QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501 -DBOOST_ALL_NO_LIB

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/boost_1_54_0/stage/release/ -llibboost_system -llibboost_thread
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/boost_1_54_0/stage/debug/ -llibboost_system -llibboost_thread
else:unix: LIBS += -L$$PWD/boost_1_54_0/stage/ -llibboost_system -llibboost_thread

INCLUDEPATH += $$PWD/boost_1_54_0
DEPENDPATH += $$PWD/boost_1_54_0

