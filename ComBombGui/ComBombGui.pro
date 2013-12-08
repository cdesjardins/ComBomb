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
    cblistwidget.cpp

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
    cblistwidget.h

FORMS    += mainwindow.ui \
    childform.ui \
    opendialog.ui \
    aboutdialog.ui \
    fileclipboarddialog.ui \
    configdialog.ui \
    runprocessdialog.ui

RESOURCES += \
    ComBomb.qrc

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..




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

QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../boost_1_54_0/stage/release/ -llibboost_system -llibboost_thread -llibboost_chrono -llibboost_date_time -llibboost_serialization
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../boost_1_54_0/stage/debug/ -llibboost_system -llibboost_thread -llibboost_chrono -llibboost_date_time -llibboost_serialization
else:unix: LIBS += $$PWD/../boost_1_54_0/stage/x86/lib/libboost_system.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_thread.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_chrono.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_date_time.a $$PWD/../boost_1_54_0/stage/x86/lib/libboost_serialization.a

INCLUDEPATH += $$PWD/../boost_1_54_0
DEPENDPATH += $$PWD/../boost_1_54_0

OTHER_FILES += \
    ComBomb.rc
