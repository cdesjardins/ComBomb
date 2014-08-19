/*
    This file is part of Konsole, an X terminal.
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

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
#include "History.h"

// System
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

// Reasonable line size
#define LINE_SIZE   1024

// History Scroll abstract base class //////////////////////////////////////

HistoryScroll::HistoryScroll()
{
}

HistoryScroll::~HistoryScroll()
{
}

bool HistoryScroll::hasScroll()
{
    return true;
}

// History Scroll Buffer //////////////////////////////////////
HistoryScrollBuffer::HistoryScrollBuffer(unsigned int maxLineCount)
    : HistoryScroll()
    , _historyBuffer()
    , _maxLineCount(0)
    , _growCount(maxLineCount)
    , _usedLines(0)
{
    growScrollback();
}

HistoryScrollBuffer::~HistoryScrollBuffer()
{

}

void HistoryScrollBuffer::clearHistory()
{
    for (int index = 0; index < _usedLines; index++)
    {
        _historyBuffer[index].clear();
        _wrappedLine[index] = false;
    }
    _maxLineCount = 0;
    _usedLines = 0;
    growScrollback();
}

void HistoryScrollBuffer::addCellsVector(const QVector<Character>& cells)
{
    _usedLines++;
    if (_usedLines >= _maxLineCount)
    {
        growScrollback();
    }

    int index = _usedLines - 1;
    _historyBuffer[index] = cells;
    _wrappedLine[index] = false;
}

void HistoryScrollBuffer::addLine(bool previousWrapped)
{
    _wrappedLine[_usedLines - 1] = previousWrapped;
}

int HistoryScrollBuffer::getLines()
{
    return _usedLines;
}

int HistoryScrollBuffer::getLineLen(int lineNumber)
{
    Q_ASSERT(lineNumber >= 0 && lineNumber < _maxLineCount);

    if (lineNumber < _usedLines)
    {
        return _historyBuffer[lineNumber].size();
    }
    else
    {
        return 0;
    }
}

bool HistoryScrollBuffer::isWrappedLine(int lineNumber)
{
    Q_ASSERT(lineNumber >= 0 && lineNumber < _maxLineCount);

    if (lineNumber < _usedLines)
    {
        return _wrappedLine[lineNumber];
    }
    else
    {
        return false;
    }
}

void HistoryScrollBuffer::getCells(int lineNumber, int startColumn, int count, Character* buffer)
{
    if (count == 0)
    {
        return;
    }

    Q_ASSERT(lineNumber < _maxLineCount);

    if (lineNumber >= _usedLines)
    {
        memset(buffer, 0, count * sizeof(Character));
        return;
    }

    const QVector<Character>& line = _historyBuffer[lineNumber];

    Q_ASSERT(startColumn <= line.size() - count);

    memcpy(buffer, line.constData() + startColumn, count * sizeof(Character));
}

void HistoryScrollBuffer::growScrollback()
{
    _maxLineCount += _growCount;
    _historyBuffer.resize(_maxLineCount);
    _wrappedLine.resize(_maxLineCount);
}

