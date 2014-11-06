#-------------------------------------------------
#
# Project created by QtCreator 2013-10-04T00:31:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
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
    finddialog.cpp

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
    finddialog.h

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

INCLUDEPATH += $$PWD/.. $$PWD/../QueuePtr/include
DEPENDPATH += $$PWD/..  $$PWD/../QueuePtr/include


# QTerminal
win32:RC_FILE = ComBomb.rc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QTerminal/release/ -lQTerminal
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QTerminal/debug/ -lQTerminal
else:unix: LIBS += $$OUT_PWD/../QTerminal/libQTerminal.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTerminal/release/QTerminal.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QTerminal/debug/QTerminal.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../QTerminal/libQTerminal.a


# Targetconnection

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TargetConnection/release/ -lTargetConnection
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TargetConnection/debug/ -lTargetConnection
else:unix: LIBS += $$OUT_PWD/../TargetConnection/libTargetConnection.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TargetConnection/release/TargetConnection.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TargetConnection/debug/TargetConnection.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../TargetConnection/libTargetConnection.a


# To build cryptLib for windows:
# I have used version: 3.4.2
# I changed the build type to static lib
# I also had to add the -DSTATIC_LIB to the cryptlib project settings
# Then build both debug and release.
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../cl/release/ -lcl32 -ladvapi32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../cl/debug/ -lcl32 -ladvapi32
else:unix: LIBS += $$PWD/../cl/libcl.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../cl/release/cl32.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../cl/debug/cl32.lib



# Boost
BOOSTVER = 1_57_0
QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../boost_$${BOOSTVER}/stage/release/ -llibboost_system -llibboost_thread -llibboost_chrono
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../boost_$${BOOSTVER}/stage/debug/ -llibboost_system -llibboost_thread -llibboost_chrono
else:unix: LIBS += $$PWD/../boost_$${BOOSTVER}/stage/lib/libboost_system.a $$PWD/../boost_$${BOOSTVER}/stage/lib/libboost_thread.a $$PWD/../boost_$${BOOSTVER}/stage/lib/libboost_chrono.a

INCLUDEPATH += $$PWD/../boost_$${BOOSTVER}
DEPENDPATH += $$PWD/../boost_$${BOOSTVER}

OTHER_FILES += \
    ComBomb.rc



win32: {
#QMAKE_CXXFLAGS_RELEASE -= -MD
#QMAKE_CXXFLAGS_RELEASE += -MT
#QMAKE_CXXFLAGS_DEBUG -= -MDd
#QMAKE_CXXFLAGS_DEBUG += -MTd
#QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:msvcrtd.lib
#QMAKE_LFLAGS_RELEASE += /NODEFAULTLIB:msvcrt.lib
}



