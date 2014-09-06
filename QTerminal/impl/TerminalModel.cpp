/*
    This file is part of Konsole

    Copyright (C) 2006-2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008
    Copyright (C) 2012 Jacob Dawid <jacob.dawid@googlemail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "TerminalModel.h"

// Standard
#include <assert.h>
#include <stdlib.h>

// Qt
#include <QApplication>
#include <QtCore/QByteRef>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore>

#include "TerminalView.h"
#include "Vt102Emulation.h"

Q_DECLARE_METATYPE(boost::intrusive_ptr<RefCntBuffer>);

TerminalModel::TerminalModel(const boost::shared_ptr<TgtIntf> &targetInterface) :
    _emulation(0)
    , _monitorActivity(false)
    , _monitorSilence(false)
    , _notifiedActivity(false)
    , _silenceSeconds(10)
    , _addToUtmp(false)
    , _fullScripting(false)
    , _hasDarkBackground(false)
    , _targetInterface(targetInterface)
    , _closed(false)
{
    qRegisterMetaType<boost::intrusive_ptr<RefCntBuffer> >();
    //create emulation backend
    _emulation.reset(new Vt102Emulation());
    connect(_emulation.get(), SIGNAL(stateSet(int)), this, SLOT(activityStateSet(int)));
    connect(_emulation.get(), SIGNAL(changeTabTextColorRequest(int)), this, SIGNAL(changeTabTextColorRequest(int)));
    connect(_emulation.get(), SIGNAL(profileChangeCommandReceived(const QString &)),  this, SIGNAL(profileChangeCommandReceived(const QString &)));

    _selfListener.reset(new SelfListener(targetInterface));
    _selfListener->start();

    connectToRecvText(this);

    connect(_emulation.get(), SIGNAL(sendData(const char*, int)) , this, SLOT(sendData(const char*, int)));

    //setup timer for monitoring session activity
    _monitorTimer.reset(new QTimer(this));
    _monitorTimer->setSingleShot(true);
    connect(_monitorTimer.get(), SIGNAL(timeout()), this, SLOT(monitorTimerDone()));
}

void TerminalModel::connectToRecvText(QObject *who)
{
    connect(_selfListener.get(), SIGNAL(recvData(boost::intrusive_ptr<RefCntBuffer>)), who, SLOT(onReceiveBlock(boost::intrusive_ptr<RefCntBuffer>)));
}

void TerminalModel::setDarkBackground(bool darkBackground)
{
    _hasDarkBackground = darkBackground;
}

bool TerminalModel::hasDarkBackground() const
{
    return _hasDarkBackground;
}

void TerminalModel::setCodec(QTextCodec* codec)
{
    _emulation->setCodec(codec);
}

void TerminalModel::addView(const boost::shared_ptr<TerminalView> &widget)
{
    Q_ASSERT(!_views.contains(widget));

    _views.append(widget);

    if (_emulation != 0)
    {
        // connect emulation - view signals and slots
        connect(widget.get(), SIGNAL(keyPressedSignal(QKeyEvent*)), _emulation.get(),
                SLOT(sendKeyEvent(QKeyEvent*)));
        connect(widget.get(), SIGNAL(mouseSignal(int, int, int, int)), _emulation.get(),
                SLOT(sendMouseEvent(int, int, int, int)));
        connect(widget.get(), SIGNAL(sendStringToEmu(const char*)), _emulation.get(),
                SLOT(sendString(const char*)));

        // allow emulation to notify view when the foreground process
        // indicates whether or not it is interested in mouse signals
        connect(_emulation.get(), SIGNAL(programUsesMouseChanged(bool)), widget.get(),
                SLOT(setUsesMouse(bool)));

        widget->setUsesMouse(_emulation->programUsesMouse());

        widget->setScreenWindow(_emulation->createWindow());
    }

    //connect view signals and slots
    QObject::connect(widget.get(), SIGNAL(changedContentSizeSignal(int, int)), this,
                     SLOT(onViewSizeChange(int, int)));

    //slot for close
    //QObject::connect(this, SIGNAL(finished()), widget, SLOT(close()));
}

void TerminalModel::sendData(const char* buf, int len)
{
    _targetInterface->tgtWrite(buf, len);
}

void TerminalModel::run()
{
    emit started();
}

void TerminalModel::monitorTimerDone()
{
    //FIXME: The idea here is that the notification popup will appear to tell the user than output from
    //the terminal has stopped and the popup will disappear when the user activates the session.
    //
    //This breaks with the addition of multiple views of a session.  The popup should disappear
    //when any of the views of the session becomes active

    //FIXME: Make message text for this notification and the activity notification more descriptive.
    if (_monitorSilence)
    {
        //    KNotification::event("Silence", ("Silence in session '%1'", _nameTitle), QPixmap(),
        //                    QApplication::activeWindow(),
        //                    KNotification::CloseWhenWidgetActivated);
        emit stateChanged(NOTIFYSILENCE);
    }
    else
    {
        emit stateChanged(NOTIFYNORMAL);
    }

    _notifiedActivity = false;
}

void TerminalModel::activityStateSet(int state)
{
    if (state == NOTIFYBELL)
    {
        emit bellRequest("");
    }
    else if (state == NOTIFYACTIVITY)
    {
        if (_monitorSilence)
        {
            _monitorTimer->start(_silenceSeconds * 1000);
        }

        if (_monitorActivity)
        {
            //FIXME:  See comments in Session::monitorTimerDone()
            if (!_notifiedActivity)
            {
                //        KNotification::event("Activity", ("Activity in session '%1'", _nameTitle), QPixmap(),
                //                        QApplication::activeWindow(),
                //        KNotification::CloseWhenWidgetActivated);
                _notifiedActivity = true;
            }
        }
    }

    if (state == NOTIFYACTIVITY && !_monitorActivity)
    {
        state = NOTIFYNORMAL;
    }
    if (state == NOTIFYSILENCE && !_monitorSilence)
    {
        state = NOTIFYNORMAL;
    }

    emit stateChanged(state);
}

void TerminalModel::onViewSizeChange(int /*height*/, int /*width*/)
{
    updateTerminalSize();
}

void TerminalModel::onEmulationSizeChange(int lines, int columns)
{
    setSize(QSize(lines, columns));
}

void TerminalModel::updateTerminalSize()
{
    QListIterator<boost::shared_ptr<TerminalView> > viewIter(_views);

    int minLines = -1;
    int minColumns = -1;

    // minimum number of lines and columns that views require for
    // their size to be taken into consideration ( to avoid problems
    // with new view widgets which haven't yet been set to their correct size )
    const int VIEW_LINES_THRESHOLD = 2;
    const int VIEW_COLUMNS_THRESHOLD = 2;

    //select largest number of lines and columns that will fit in all visible views
    while (viewIter.hasNext())
    {
        boost::shared_ptr<TerminalView> view = viewIter.next();
        if (view->isHidden() == false &&
            view->lines() >= VIEW_LINES_THRESHOLD &&
            view->columns() >= VIEW_COLUMNS_THRESHOLD)
        {
            minLines = (minLines == -1) ? view->lines() : qMin(minLines, view->lines());
            minColumns = (minColumns == -1) ? view->columns() : qMin(minColumns, view->columns());
        }
    }

    // backend emulation must have a _terminal of at least 1 column x 1 line in size
    if (minLines > 0 && minColumns > 0)
    {
        _emulation->setImageSize(minLines, minColumns);
        //_kpty->setWinSize (minLines, minColumns);
        //_shellProcess->setWindowSize( minLines , minColumns );
    }
}

void TerminalModel::refresh()
{
}

void TerminalModel::closeEvent(QCloseEvent * )
{
    _closed = true;
    _targetInterface->tgtDisconnect();
}

void TerminalModel::sendText(const QString &text) const
{
    if (_closed == false)
    {
        _emulation->sendText(text);
    }
}


void TerminalModel::sendText(const QByteArray &text) const
{
    if (_closed == false)
    {
        _emulation->sendText(text);
    }
}

void TerminalModel::onReceiveBlock(boost::intrusive_ptr<RefCntBuffer> incoming)
{
    if (_closed == false)
    {
        char* buf = boost::asio::buffer_cast<char*>(incoming->_buffer);
        int len = boost::asio::buffer_size(incoming->_buffer);
        _emulation->receiveData(buf, len);
    }
}

TerminalModel::~TerminalModel()
{
    _targetInterface.reset();
    _selfListener->join();
    _selfListener.reset();
    _emulation.reset();
    _views.clear();
}

void TerminalModel::setProfileKey(const QString& key)
{
    _profileKey = key;
    emit profileChanged(key);
}

QString TerminalModel::profileKey() const
{
    return _profileKey;
}

void TerminalModel::done(int)
{
    emit finished();
}

QString TerminalModel::keyBindings() const
{
    return _emulation->keyBindings();
}

void TerminalModel::setKeyBindings(const QString &id)
{
    _emulation->setKeyBindings(id);
}

void TerminalModel::clearHistory()
{
    _emulation->clearHistory();
}

void TerminalModel::clearScreen()
{
    _emulation->clearEntireScreen();
    _emulation->clearHistory();
    _emulation->home();
}

// unused currently
bool TerminalModel::isMonitorActivity() const
{
    return _monitorActivity;
}

// unused currently
bool TerminalModel::isMonitorSilence()  const
{
    return _monitorSilence;
}

void TerminalModel::setMonitorActivity(bool _monitor)
{
    _monitorActivity = _monitor;
    _notifiedActivity = false;

    activityStateSet(NOTIFYNORMAL);
}

void TerminalModel::setMonitorSilence(bool _monitor)
{
    if (_monitorSilence == _monitor)
    {
        return;
    }

    _monitorSilence = _monitor;
    if (_monitorSilence)
    {
        _monitorTimer->start(_silenceSeconds * 1000);
    }
    else
    {
        _monitorTimer->stop();
    }

    activityStateSet(NOTIFYNORMAL);
}

void TerminalModel::setMonitorSilenceSeconds(int seconds)
{
    _silenceSeconds = seconds;
    if (_monitorSilence)
    {
        _monitorTimer->start(_silenceSeconds * 1000);
    }
}

void TerminalModel::setAddToUtmp(bool set)
{
    _addToUtmp = set;
}

QSize TerminalModel::size()
{
    return _emulation->imageSize();
}

void TerminalModel::setSize(const QSize& size)
{
    if ((size.width() <= 1) || (size.height() <= 1))
    {
        return;
    }

    emit resizeRequest(size);
}

void TerminalModel::newlineToggle()
{
    _emulation->newlineToggle();
}

int TerminalModel::lineCount()
{
    return _emulation->lineCount();
}
