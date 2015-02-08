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
    TgtProcessConnection.cpp \
    TgtThread.cpp

HEADERS += \
    TgtTelnetConnection.h \
    TgtSshConnection.h \
    TgtSerialConnection.h \
    TgtConnection.h \
    CBException.h \
    TgtProcessConnection.h \
    TgtThread.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

INCLUDEPATH += $$PWD/.. $$PWD/../QueuePtr/include
DEPENDPATH  += $$PWD/.. $$PWD/../QueuePtr/include

# Boost

QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501
BOOSTVER = 1_57_0
INCLUDEPATH += $$PWD/../boost_$${BOOSTVER}
DEPENDPATH += $$PWD/../boost_$${BOOSTVER}

# Crypt lib
win32: QMAKE_CXXFLAGS += -D_WINDOWS -DSTATIC_LIB

unix: {
QMAKE_CXXFLAGS += -std=c++11
}
win32: {
#QMAKE_CXXFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS_RELEASE += -MT
#QMAKE_CXXFLAGS_DEBUG -= -MDd
#QMAKE_CXXFLAGS_DEBUG += -MTd
#QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:msvcrtd.lib
#QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:msvcrt.lib
}
