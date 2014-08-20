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

bool HistoryScroll::hasScroll()
{
    return true;
}

HistoryScroll::HistoryScroll()
{
}

HistoryScroll::~HistoryScroll()
{
}

void HistoryScroll::clearHistory()
{
    _historyBuffer.clear();
    _wrappedLine.clear();
}

void HistoryScroll::addCellsVector(const std::vector<Character>& cells, bool previousWrapped)
{
    _historyBuffer.push_back(cells);
    _wrappedLine.resize(_historyBuffer.size());
    _wrappedLine[_wrappedLine.size() - 1] = previousWrapped;
}

int HistoryScroll::getLines()
{
    return _historyBuffer.size();
}

int HistoryScroll::getLineLen(size_t lineNumber)
{
    Q_ASSERT(lineNumber >= 0 && lineNumber < _historyBuffer.size());

    if (lineNumber < _historyBuffer.size())
    {
        return _historyBuffer[lineNumber].size();
    }
    else
    {
        return 0;
    }
}

bool HistoryScroll::isWrappedLine(int lineNumber)
{
    Q_ASSERT(lineNumber >= 0 && lineNumber < _wrappedLine.size());

    if (lineNumber < _wrappedLine.size())
    {
        return _wrappedLine[lineNumber];
    }
    else
    {
        return false;
    }
}

void HistoryScroll::getCells(size_t lineNumber, int startColumn, int count, std::vector<Character>::iterator buffer, Character defaultChar)
{
    if (count == 0)
    {
        return;
    }

    //Q_ASSERT(lineNumber < _historyBuffer.size());

    if (lineNumber >= _historyBuffer.size())
    {
        for (int index = 0; index < count; index++)
        {
            buffer[index] = defaultChar;
        }
        return;
    }

    const std::vector<Character>& line = _historyBuffer[lineNumber];

    Q_ASSERT(startColumn <= line.size() - count);

    for (int index = 0; index < count; index++)
    {
        buffer[index] = line[index + startColumn];
    }
    //memcpy(buffer, line.constData() + startColumn, count * sizeof(Character));
}


