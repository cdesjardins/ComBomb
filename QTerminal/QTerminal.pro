#-------------------------------------------------
#
# Project created by QtCreator 2013-10-04T00:32:37
#
#-------------------------------------------------

QT       += gui core widgets

TARGET = QTerminal
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    impl/Vt102Emulation.cpp \
    impl/TerminalView.cpp \
    impl/TerminalModel.cpp \
    impl/TerminalCharacterDecoder.cpp \
    impl/SelfListener.cpp \
    impl/ScreenWindow.cpp \
    impl/Screen.cpp \
    impl/QTerminalImpl.cpp \
    impl/konsole_wcwidth.cpp \
    impl/History.cpp \
    impl/Emulation.cpp \
    impl/KeyboardTranslator.cpp \
    impl/TgtIntf.cpp \
    impl/QTerminalConfig.cpp \
    ../QueuePtr/src/RefCntBufferPool.cpp \
    impl/CharacterColor.cpp

HEADERS += QTerminalInterface.h \
    qterminal.h \
    QTerminal \
    impl/Vt102Emulation.h \
    impl/TerminalView.h \
    impl/TerminalModel.h \
    impl/TerminalCharacterDecoder.h \
    impl/SelfListener.h \
    impl/ScreenWindow.h \
    impl/Screen.h \
    impl/QTerminalImpl.h \
    impl/LineFont.h \
    impl/konsole_wcwidth.h \
    impl/History.h \
    impl/Emulation.h \
    impl/CharacterColor.h \
    impl/Character.h \
    TgtIntf.h \
    impl/KeyboardTranslator.h \
    impl/ExtendedDefaultTranslator.h \
    QTerminalConfig.h \
    impl/BackTabEvent.h \
    ../QueuePtr/include/IntrusivePtrBase.h \
    ../QueuePtr/include/RefCntBuffer.h \
    ../QueuePtr/include/RefCntBufferPool.h \
    ../QueuePtr/include/ThreadSafePool.h \
    ../QueuePtr/include/ThreadSafeQueue.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}


INCLUDEPATH += $$PWD/.. $$PWD/../QTerminal $$PWD/../QueuePtr/include
DEPENDPATH  += $$PWD/.. $$PWD/../QTerminal $$PWD/../QueuePtr/include

# Boost

QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501
BOOSTVER = 1_57_0
INCLUDEPATH += $$PWD/../boost_$${BOOSTVER}
DEPENDPATH += $$PWD/../boost_$${BOOSTVER}

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
