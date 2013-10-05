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
    impl/TgtIntf.cpp

HEADERS += qterminal.h \
    QTerminalInterface.h \
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
    impl/ThreadSafeQueue.h \
    impl/KeyboardTranslator.h \
    impl/ExtendedDefaultTranslator.h
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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../boost_1_54_0/stage/release/ -llibboost_system -llibboost_thread -llibboost_chrono -llibboost_date_time
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../boost_1_54_0/stage/debug/ -llibboost_system -llibboost_thread -llibboost_chrono -llibboost_date_time
else:unix: LIBS += $$PWD/../boost_1_54_0/stage/x86/lib/libboost_system.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_thread.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_chrono.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_date_time.a

INCLUDEPATH += $$PWD/../boost_1_54_0
DEPENDPATH += $$PWD/../boost_1_54_0
