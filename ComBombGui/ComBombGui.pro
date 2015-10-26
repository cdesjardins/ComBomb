#-------------------------------------------------
#
# Project created by QtCreator 2013-10-04T00:31:16
#
#-------------------------------------------------

QT       += core gui widgets network

CONFIG += static
TARGET = ComBombGui
TEMPLATE = app

versionTarget.target = createVersion
versionTarget.depends = $$PWD/../.git
versionTarget.commands = cd $$PWD && $$PWD/../createVersion.py

QMAKE_EXTRA_TARGETS += versionTarget
PRE_TARGETDEPS += createVersion
DEPENDPATH += .

SOURCES += main.cpp\
        mainwindow.cpp \
    childform.cpp \
    opendialog.cpp \
    aboutdialog.cpp \
    versioning.cpp \
    fileclipboarddialog.cpp \
    configdialog.cpp \
    runprocessdialog.cpp \
    cbcombobox.cpp \
    cblistwidget.cpp \
    cbdialog.cpp \
    finddialog.cpp \
    updatechecker.cpp \
    cbfontcombobox.cpp \
    capturedialog.cpp

HEADERS  += mainwindow.h \
    childform.h \
    opendialog.h \
    aboutdialog.h \
    versioning.h \
    cblabel.h \
    fileclipboarddialog.h \
    fileclipboardheader.h \
    configdialog.h \
    runprocessdialog.h \
    cbcombobox.h \
    cblistwidget.h \
    cbdialog.h \
    finddialog.h \
    updatechecker.h \
    cbfontcombobox.h \
    capturedialog.h

FORMS    += mainwindow.ui \
    childform.ui \
    opendialog.ui \
    aboutdialog.ui \
    fileclipboarddialog.ui \
    configdialog.ui \
    runprocessdialog.ui \
    finddialog.ui \
    capturedialog.ui

RESOURCES += \
    ComBomb.qrc

INCLUDEPATH += $$PWD/../../install/include $$PWD/../../include
DEPENDPATH += $$PWD/../../install/include $$PWD/../../include




# Targetconnection

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TargetConnection/release/ -lTargetConnection
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TargetConnection/debug/ -lTargetConnection
else:unix: LIBS += $$OUT_PWD/../TargetConnection/libTargetConnection.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TargetConnection/release/TargetConnection.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TargetConnection/debug/TargetConnection.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../TargetConnection/libTargetConnection.a

# QTerminal
win32:RC_FILE = ComBomb.rc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QTerminal/release/ -lQTerminal
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QTerminal/debug/ -lQTerminal
else:unix: LIBS += $$OUT_PWD/../QTerminal/libQTerminal.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTerminal/release/QTerminal.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTerminal/debug/QTerminal.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../QTerminal/libQTerminal.a



# Boost
QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB -DBOOST_NO_AUTO_PTR

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../external/boost/install/lib/release/ -llibboost_system
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../external/boost/install/lib/debug/ -llibboost_system
else:unix: LIBS += $$PWD/../../external/boost/install/lib/libboost_system.a

INCLUDEPATH += $$PWD/../../external/boost/install/include
DEPENDPATH += $$PWD/../../external/boost/install/include

OTHER_FILES += \
    ComBomb.rc

# QueuePtr
win32:CONFIG(release, debug|release):     LIBS += -L$$PWD/../../install/lib/ -lQueuePtr
else:win32:CONFIG(debug, debug|release):  LIBS += -L$$PWD/../../install/lib/ -lQueuePtrd
else:unix:CONFIG(release, debug|release): LIBS += $$PWD/../../install/lib/libQueuePtr.a
else:unix:CONFIG(debug, debug|release):   LIBS += $$PWD/../../install/lib/libQueuePtrd.a

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

# Cppssh
QMAKE_CXXFLAGS += -DCPPSSH_STATIC
win32:CONFIG(release, debug|release):     LIBS += -L$$PWD/../../botan/install/lib/botan/release -lbotan -lCDLogger -lcppssh
else:win32:CONFIG(debug, debug|release):  LIBS += -L$$PWD/../../botan/install/lib/botan/debug   -lbotan -lCDLoggerd -lcppsshd
else:unix:CONFIG(release, debug|release): LIBS += $$PWD/../../install/lib/libcppssh.a  $$PWD/../../install/lib/libCDLogger.a  $$PWD/../../botan/install/lib/botan/release/libbotan-1.11.a
else:unix:CONFIG(debug, debug|release):   LIBS += $$PWD/../../install/lib/libcppsshd.a $$PWD/../../install/lib/libCDLoggerd.a $$PWD/../../botan/install/lib/botan/debug/libbotan-1.11.a

win32:CONFIG(release, debug|release):     PRE_TARGETDEPS += $$PWD/../../install/lib/cppssh.lib   $$PWD/../../install/lib/CDLogger.lib   $$PWD/../../botan/install/lib/botan/release/botan.lib
else:win32:CONFIG(debug, debug|release):  PRE_TARGETDEPS += $$PWD/../../install/lib/cppsshd.lib  $$PWD/../../install/lib/CDLoggerd.lib  $$PWD/../../botan/install/lib/botan/debug/botan.lib
else:unix:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../install/lib/libcppssh.a  $$PWD/../../install/lib/libCDLogger.a  $$PWD/../../botan/install/lib/botan/release/libbotan-1.11.a
else:unix:CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$PWD/../../install/lib/libcppsshd.a $$PWD/../../install/lib/libCDLoggerd.a $$PWD/../../botan/install/lib/botan/debug/libbotan-1.11.a

unix: LIBS += -static-libstdc++
