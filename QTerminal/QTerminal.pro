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
    impl/Emulation.cpp

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
    impl/Character.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
