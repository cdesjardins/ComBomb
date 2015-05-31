#-------------------------------------------------
#
# Project created by QtCreator 2013-10-04T00:31:16
#
#-------------------------------------------------

QT       += core gui widgets network

CONFIG += static
TARGET = ComBombGui
TEMPLATE = app

versionTarget.target = versioning.h
versionTarget.depends = $$PWD/../.git
versionTarget.commands = $$PWD/../createVersion.py
PRE_TARGETDEPS += versioning.h
QMAKE_EXTRA_TARGETS += versionTarget
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
    cbfontcombobox.cpp

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
    cbfontcombobox.h

FORMS    += mainwindow.ui \
    childform.ui \
    opendialog.ui \
    aboutdialog.ui \
    fileclipboarddialog.ui \
    configdialog.ui \
    runprocessdialog.ui \
    finddialog.ui

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

# To build cryptLib for windows:
# I have used version: 3.4.2
# I changed the build type to static lib
# I also had to add the -DSTATIC_LIB to the cryptlib project settings
# Then build both debug and release.
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../cryptlib/release/ -ladvapi32 -lcl32 -luser32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../cryptlib/debug/ -ladvapi32 -lcl32 -luser32
else:unix: LIBS += $$PWD/../../cryptlib/libcl.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../cryptlib/release/cl32.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../cryptlib/debug/cl32.lib



# Boost
BOOSTVER = 1_58_0
QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../boost/boost_$${BOOSTVER}/stage/release/ -llibboost_system
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../boost/boost_$${BOOSTVER}/stage/debug/ -llibboost_system
else:unix: LIBS += $$PWD/../../boost/boost_$${BOOSTVER}/stage/lib/libboost_system.a

INCLUDEPATH += $$PWD/../../boost/boost_$${BOOSTVER}
DEPENDPATH += $$PWD/../../boost/boost_$${BOOSTVER}

OTHER_FILES += \
    ComBomb.rc

# QueuePtr
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../install/lib/ -lQueuePtr
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../install/lib/ -lQueuePtrd
else:unix: LIBS += $$PWD/../../install/lib/libQueuePtr.a

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

win32:CONFIG(release, debug|release): LIBS += -lbotan-1.11 -lCDLogger -lcppssh
else:win32:CONFIG(debug, debug|release): LIBS += -lbotan-1.11 -lCDLoggerd -lcppsshd
else:unix: LIBS += $$PWD/../../install/lib/libcppssh.a $$PWD/../../install/lib/libCDLogger.a $$PWD/../../install/lib/libbotan-1.11.a


