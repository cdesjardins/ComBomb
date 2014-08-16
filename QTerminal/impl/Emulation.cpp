/*
    This file is part of Konsole, an X terminal.

    Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
    Copyright (C) 1996 by Matthias Ettrich <ettrich@kde.org>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

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
#include "Emulation.h"

// System
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Qt
#include <QApplication>
#include <QtGui/QClipboard>
#include <QtCore/QHash>
#include <QtGui/QKeyEvent>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>
#include <QtCore/QThread>

#include <QtCore/QTime>

// Konsole
#include "KeyboardTranslator.h"
#include "Screen.h"
#include "TerminalCharacterDecoder.h"
#include "ScreenWindow.h"

Emulation::Emulation() :
    _currentScreenIndex(0),
    _codec(0),
    _decoder(NULL),
    _usesMouse(false)
{
    QObject::connect(&_bulkTimer1, SIGNAL(timeout()), this, SLOT(showBulk()));
    QObject::connect(&_bulkTimer2, SIGNAL(timeout()), this, SLOT(showBulk()));

    boost::shared_ptr<HistoryScroll> hist(new HistoryScrollBuffer(10000));
    _screen.append(boost::shared_ptr<Screen>(new Screen(hist)));
    _screen.append(boost::shared_ptr<Screen>(new Screen(hist)));

    // listen for mouse status changes
    connect(this, SIGNAL(programUsesMouseChanged(bool)),
            SLOT(usesMouseChanged(bool)));
}

bool Emulation::programUsesMouse() const
{
    return _usesMouse;
}

void Emulation::usesMouseChanged(bool usesMouse)
{
    _usesMouse = usesMouse;
}

ScreenWindow* Emulation::createWindow()
{
    boost::shared_ptr<ScreenWindow> window(new ScreenWindow());
    window->setScreen(_screen[_currentScreenIndex].get());
    _windows << window;

    connect(window.get(), SIGNAL(selectionChanged()),
            this, SLOT(bufferedUpdate()));

    connect(this, SIGNAL(outputChanged()),
            window.get(), SLOT(notifyOutputChanged()));
    return window.get();
}

/*!
*/

Emulation::~Emulation()
{
    _windows.clear();

    if (_decoder != NULL)
    {
        delete _decoder;
        _decoder = NULL;
    }
}

/*! change between primary and alternate _screen
*/

void Emulation::setScreen(int n)
{
    int old = _currentScreenIndex;
    _currentScreenIndex = n & 1;
    if (_currentScreenIndex != old)
    {
        _screen[old]->setBusySelecting(false);

        // tell all windows onto this emulation to switch to the newly active _screen
        QListIterator<boost::shared_ptr<ScreenWindow> > windowIter(_windows);
        while (windowIter.hasNext())
        {
            windowIter.next()->setScreen(_screen[_currentScreenIndex].get());
        }
    }
}

void Emulation::clearHistory()
{
    _screen[_currentScreenIndex]->clearHistory();
}

void Emulation::home()
{
    _screen[_currentScreenIndex]->home();
}

void Emulation::setCodec(const QTextCodec* qtc)
{
    Q_ASSERT(qtc);

    _codec = qtc;
    if (_decoder != NULL)
    {
        delete _decoder;
        _decoder = NULL;
    }
    _decoder = _codec->makeDecoder();

    emit useUtf8Request(utf8());
}

void Emulation::setCodec(EmulationCodec codec)
{
    if (codec == Utf8Codec)
    {
        setCodec(QTextCodec::codecForName("utf8"));
    }
    else if (codec == LocaleCodec)
    {
        setCodec(QTextCodec::codecForLocale());
    }
}

void Emulation::setKeyBindings(const QString& name)
{
    _keyTranslator = KeyboardTranslatorManager::instance()->findTranslator(name);
}

QString Emulation::keyBindings()
{
    return _keyTranslator->name();
}

// Interpreting Codes ---------------------------------------------------------

/*
   This section deals with decoding the incoming character stream.
   Decoding means here, that the stream is first separated into `tokens'
   which are then mapped to a `meaning' provided as operations by the
   `Screen' class.
*/

/*!
*/

void Emulation::receiveChar(int c)
// process application unicode input to terminal
// this is a trivial scanner
{
    c &= 0xff;
    switch (c)
    {
        case '\b': _screen[_currentScreenIndex]->BackSpace();                 break;
        case '\t': _screen[_currentScreenIndex]->Tabulate();                  break;
        case '\n': _screen[_currentScreenIndex]->NewLine();                   break;
        case '\r': _screen[_currentScreenIndex]->Return();                    break;
        case 0x07: emit stateSet(NOTIFYBELL);
            break;
        default: _screen[_currentScreenIndex]->ShowCharacter(c);            break;
    }
    ;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                             Keyboard Handling                             */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/*!
*/

void Emulation::sendKeyEvent(QKeyEvent* ev)
{
    emit stateSet(NOTIFYNORMAL);

    if (!ev->text().isEmpty())
    { // A block of text
      // Note that the text is proper unicode.
      // We should do a conversion here, but since this
      // routine will never be used, we simply emit plain ascii.
      //emit sendBlock(ev->text().toAscii(),ev->text().length());
        emit sendData(ev->text().toUtf8(), ev->text().length());
    }
}

void Emulation::sendString(const char*, int)
{
    // default implementation does nothing
}

void Emulation::sendMouseEvent(int /*buttons*/, int /*column*/, int /*row*/, int /*eventType*/)
{
    // default implementation does nothing
}

// Unblocking, Byte to Unicode translation --------------------------------- --

/*
   We are doing code conversion from locale to unicode first.
TODO: Character composition from the old code.  See #96536
*/

void Emulation::receiveData(const char* text, int length)
{
    emit stateSet(NOTIFYACTIVITY);

    bufferedUpdate();

    QString unicodeText = _decoder->toUnicode(text, length);

    //send characters to terminal emulator
    for (int i = 0; i < unicodeText.length(); i++)
    {
        receiveChar(unicodeText[i].unicode());
    }
}

// Selection --------------------------------------------------------------- --

int Emulation::lineCount()
{
    // sum number of lines currently on _screen plus number of lines in history
    return _screen[_currentScreenIndex]->getLines() + _screen[_currentScreenIndex]->getHistLines();
}

// Refreshing -------------------------------------------------------------- --

#define BULK_TIMEOUT1 10
#define BULK_TIMEOUT2 40

/*!
*/
void Emulation::showBulk()
{
    _bulkTimer1.stop();
    _bulkTimer2.stop();

    emit outputChanged();

    _screen[_currentScreenIndex]->resetScrolledLines();
    _screen[_currentScreenIndex]->resetDroppedLines();
}

void Emulation::bufferedUpdate()
{
    _bulkTimer1.setSingleShot(true);
    _bulkTimer1.start(BULK_TIMEOUT1);
    if (!_bulkTimer2.isActive())
    {
        _bulkTimer2.setSingleShot(true);
        _bulkTimer2.start(BULK_TIMEOUT2);
    }
}

char Emulation::getErase() const
{
    return '\b';
}

void Emulation::setImageSize(int lines, int columns)
{
    //kDebug() << "Resizing image to: " << lines << "by" << columns << QTime::currentTime().msec();
    Q_ASSERT(lines > 0);
    Q_ASSERT(columns > 0);

    _screen[0]->resizeImage(lines, columns);
    _screen[1]->resizeImage(lines, columns);

    emit imageSizeChanged(lines, columns);

    bufferedUpdate();
}

QSize Emulation::imageSize()
{
    return QSize(_screen[_currentScreenIndex]->getColumns(), _screen[_currentScreenIndex]->getLines());
}

ushort ExtendedCharTable::extendedCharHash(ushort* unicodePoints, ushort length) const
{
    ushort hash = 0;
    for (ushort i = 0; i < length; i++)
    {
        hash = 31 * hash + unicodePoints[i];
    }
    return hash;
}

bool ExtendedCharTable::extendedCharMatch(ushort hash, ushort* unicodePoints, ushort length) const
{
    ushort* entry = extendedCharTable[hash];

    // compare given length with stored sequence length ( given as the first ushort in the
    // stored buffer )
    if (entry == 0 || entry[0] != length)
    {
        return false;
    }
    // if the lengths match, each character must be checked.  the stored buffer starts at
    // entry[1]
    for (int i = 0; i < length; i++)
    {
        if (entry[i + 1] != unicodePoints[i])
        {
            return false;
        }
    }
    return true;
}

ushort ExtendedCharTable::createExtendedChar(ushort* unicodePoints, ushort length)
{
    // look for this sequence of points in the table
    ushort hash = extendedCharHash(unicodePoints, length);

    // check existing entry for match
    while (extendedCharTable.contains(hash))
    {
        if (extendedCharMatch(hash, unicodePoints, length))
        {
            // this sequence already has an entry in the table,
            // return its hash
            return hash;
        }
        else
        {
            // if hash is already used by another, different sequence of unicode character
            // points then try next hash
            hash++;
        }
    }

    // add the new sequence to the table and
    // return that index
    ushort* buffer = new ushort[length + 1];
    buffer[0] = length;
    for (int i = 0; i < length; i++)
    {
        buffer[i + 1] = unicodePoints[i];
    }

    extendedCharTable.insert(hash, buffer);

    return hash;
}

ushort* ExtendedCharTable::lookupExtendedChar(ushort hash, ushort& length) const
{
    // lookup index in table and if found, set the length
    // argument and return a pointer to the character sequence

    ushort* buffer = extendedCharTable[hash];
    if (buffer)
    {
        length = buffer[0];
        return buffer + 1;
    }
    else
    {
        length = 0;
        return 0;
    }
}

ExtendedCharTable::ExtendedCharTable()
{
}

ExtendedCharTable::~ExtendedCharTable()
{
    // free all allocated character buffers
    QHashIterator<ushort, ushort*> iter(extendedCharTable);
    while (iter.hasNext())
    {
        iter.next();
        delete[] iter.value();
    }
}

// global instance
ExtendedCharTable ExtendedCharTable::instance;
