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
    TgtSerialConnection.cpp \
    TgtProcessConnection.cpp \
    TgtThread.cpp \
    TgtCppsshConnection.cpp

HEADERS += \
    TgtTelnetConnection.h \
    TgtSerialConnection.h \
    TgtConnection.h \
    CBException.h \
    TgtProcessConnection.h \
    TgtThread.h \
    TgtCppsshConnection.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../../install/include $$PWD/../../include
DEPENDPATH  += $$PWD/../../install/include $$PWD/../../include

# Boost

QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB -DBOOST_NO_AUTO_PTR

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501
INCLUDEPATH += $$PWD/../../external/boost/install/include
DEPENDPATH += $$PWD/../../external/boost/install/include


unix: {
QMAKE_CXXFLAGS += -std=c++11
}
win32: {
QMAKE_CXXFLAGS_RELEASE -= -MD
QMAKE_CXXFLAGS_RELEASE += -MT
QMAKE_CXXFLAGS_DEBUG -= -MDd
QMAKE_CXXFLAGS_DEBUG += -MTd
QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:msvcrtd.lib
QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:msvcrt.lib
}

# cppssh
QMAKE_CXXFLAGS += -DCPPSSH_STATIC
