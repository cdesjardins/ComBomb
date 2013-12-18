#-------------------------------------------------
#
# Project created by QtCreator 2013-10-06T10:50:46
#
#-------------------------------------------------

QT       -= gui
QT       += widgets

TARGET = TargetConnection
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    TgtTelnetConnection.cpp \
    TgtSshConnection.cpp \
    TgtSerialConnection.cpp \
    TgtFileConnection.cpp

HEADERS += \
    TgtTelnetConnection.h \
    TgtSshConnection.h \
    TgtSerialConnection.h \
    TgtFileConnection.h \
    TgtConnection.h \
    CBException.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

# Boost

QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501

INCLUDEPATH += $$PWD/../boost_1_55_0
DEPENDPATH += $$PWD/../boost_1_55_0

# Crypt lib
win32: QMAKE_CXXFLAGS += -D_WINDOWS -DSTATIC_LIB
