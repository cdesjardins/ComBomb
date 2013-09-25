#-------------------------------------------------
#
# Project created by QtCreator 2013-01-27T13:05:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ComBomb
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    childform.cpp \
    cbtextedit.cpp \
    TargetIntf.cpp \
    opendialog.cpp \
    vt100.cpp \
    tgtterminal.cpp

HEADERS  += mainwindow.h \
    childform.h \
    cbtextedit.h \
    TargetIntf.h \
    opendialog.h \
    vt100.h \
    ThreadSafeQueue.h \
    tgtterminal.h

FORMS    += mainwindow.ui \
    childform.ui \
    opendialog.ui

RESOURCES += \
    ComBomb.qrc

QMAKE_CXXFLAGS += -DBOOST_ALL_NO_LIB

win32:QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/boost_1_54_0/stage/release/ -llibboost_system -llibboost_thread -llibboost_chrono -llibboost_date_time
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/boost_1_54_0/stage/debug/ -llibboost_system -llibboost_thread -llibboost_chrono -llibboost_date_time
else:unix: LIBS +=  $$PWD/boost_1_54_0/stage/x86/lib/libboost_system.a $$PWD/boost_1_54_0/stage/x86/lib/libboost_thread.a $$PWD/boost_1_54_0/stage/x86/lib/libboost_chrono.a $$PWD/boost_1_54_0/stage/x86/lib/libboost_date_time.a

INCLUDEPATH += $$PWD/boost_1_54_0
DEPENDPATH += $$PWD/boost_1_54_0

