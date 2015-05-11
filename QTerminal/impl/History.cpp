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
#ifndef WIN32
#include <malloc.h>
#endif

#include <QThread>
// Reasonable line size
#define LINE_SIZE   1024

History::History(size_t histSize)
    : _historyBuffer(histSize)
{
}

History::~History()
{
}

void History::clearHistory()
{
    _wrappedLine.clear();
    _historyBuffer.clear();
#ifndef WIN32
    // Clearing the history can free a ton of memory, but only small chunks
    // on linux this was causing the memory usage to remain high even after
    // a clear screen.
    malloc_trim(0);
#endif
}

bool History::resizeHistory(size_t histSize)
{
    bool ret = false;
    if (histSize != _historyBuffer.capacity())
    {
        ret = histSize < _historyBuffer.capacity();
        _historyBuffer.rset_capacity(histSize);
    }
    return ret;
}

void History::addCellsVector(const std::vector<Character>& cells, bool previousWrapped)
{
    _historyBuffer.push_back(cells);
    _wrappedLine.resize(_historyBuffer.size());
    _wrappedLine[_wrappedLine.size() - 1] = previousWrapped;
}

int History::getLines()
{
    return _historyBuffer.size();
}

int History::getLineLen(size_t lineNumber)
{
    Q_ASSERT(lineNumber < _historyBuffer.size());

    if (lineNumber < _historyBuffer.size())
    {
        return _historyBuffer[lineNumber].size();
    }
    else
    {
        return 0;
    }
}

bool History::isWrappedLine(size_t lineNumber)
{
    Q_ASSERT(lineNumber < (size_t)_wrappedLine.size());

    if (lineNumber < (size_t)_wrappedLine.size())
    {
        return _wrappedLine[(uint)lineNumber];
    }
    else
    {
        return false;
    }
}

void History::getCells(size_t lineNumber, int startColumn, int count, std::vector<Character>::iterator buffer, Character defaultChar)
{
    if (count == 0)
    {
        return;
    }

    if (lineNumber >= _historyBuffer.size())
    {
        for (int index = 0; index < count; index++)
        {
            buffer[index] = defaultChar;
        }
        return;
    }

    Q_ASSERT((size_t)startColumn <= _historyBuffer[lineNumber].size() - count);

    for (int index = 0; index < count; index++)
    {
        buffer[index] = _historyBuffer[lineNumber][index + startColumn];
    }
}

