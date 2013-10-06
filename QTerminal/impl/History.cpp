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

HistoryScroll::HistoryScroll(HistoryType* t)
    : m_histType(t)
{
}

HistoryScroll::~HistoryScroll()
{
    delete m_histType;
}

bool HistoryScroll::hasScroll()
{
    return true;
}

// History Scroll Buffer //////////////////////////////////////
HistoryScrollBuffer::HistoryScrollBuffer(unsigned int maxLineCount)
    : HistoryScroll(new HistoryTypeBuffer(maxLineCount))
    , _historyBuffer()
    , _maxLineCount(0)
    , _usedLines(0)
    , _head(0)
{
    setMaxNbLines(maxLineCount);
}

HistoryScrollBuffer::~HistoryScrollBuffer()
{
    delete[] _historyBuffer;
}

void HistoryScrollBuffer::addCellsVector(const QVector<Character>& cells)
{
    _head++;
    if (_usedLines < _maxLineCount)
    {
        _usedLines++;
    }

    if (_head >= _maxLineCount)
    {
        _head = 0;
    }

    _historyBuffer[bufferIndex(_usedLines - 1)] = cells;
    _wrappedLine[bufferIndex(_usedLines - 1)] = false;
}

void HistoryScrollBuffer::addCells(const Character a[], int count)
{
    HistoryLine newLine(count);
    qCopy(a, a + count, newLine.begin());

    addCellsVector(newLine);
}

void HistoryScrollBuffer::addLine(bool previousWrapped)
{
    _wrappedLine[bufferIndex(_usedLines - 1)] = previousWrapped;
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
        return _historyBuffer[bufferIndex(lineNumber)].size();
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
        //kDebug() << "Line" << lineNumber << "wrapped is" << _wrappedLine[bufferIndex(lineNumber)];
        return _wrappedLine[bufferIndex(lineNumber)];
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

    const HistoryLine& line = _historyBuffer[bufferIndex(lineNumber)];

    //kDebug() << "startCol " << startColumn;
    //kDebug() << "line.size() " << line.size();
    //kDebug() << "count " << count;

    Q_ASSERT(startColumn <= line.size() - count);

    memcpy(buffer, line.constData() + startColumn, count * sizeof(Character));
}

void HistoryScrollBuffer::setMaxNbLines(unsigned int lineCount)
{
    HistoryLine* oldBuffer = _historyBuffer;
    HistoryLine* newBuffer = new HistoryLine[lineCount];

    for (int i = 0; i < qMin(_usedLines, (int)lineCount); i++)
    {
        newBuffer[i] = oldBuffer[bufferIndex(i)];
    }

    _usedLines = qMin(_usedLines, (int)lineCount);
    _maxLineCount = lineCount;
    _head = (_usedLines == _maxLineCount) ? 0 : _usedLines - 1;

    _historyBuffer = newBuffer;
    delete[] oldBuffer;

    _wrappedLine.resize(lineCount);
}

int HistoryScrollBuffer::bufferIndex(int lineNumber)
{
    Q_ASSERT(lineNumber >= 0);
    Q_ASSERT(lineNumber < _maxLineCount);
    Q_ASSERT((_usedLines == _maxLineCount) || lineNumber <= _head);

    if (_usedLines == _maxLineCount)
    {
        return (_head + lineNumber + 1) % _maxLineCount;
    }
    else
    {
        return lineNumber;
    }
}

// History Scroll None //////////////////////////////////////

HistoryScrollNone::HistoryScrollNone()
    : HistoryScroll(new HistoryTypeNone())
{
}

HistoryScrollNone::~HistoryScrollNone()
{
}

bool HistoryScrollNone::hasScroll()
{
    return false;
}

int HistoryScrollNone::getLines()
{
    return 0;
}

int HistoryScrollNone::getLineLen(int)
{
    return 0;
}

bool HistoryScrollNone::isWrappedLine(int /*lineno*/)
{
    return false;
}

void HistoryScrollNone::getCells(int, int, int, Character[])
{
}

void HistoryScrollNone::addCells(const Character[], int)
{
}

void HistoryScrollNone::addLine(bool)
{
}

//////////////////////////////////////////////////////////////////////
// History Types
//////////////////////////////////////////////////////////////////////

HistoryType::HistoryType()
{
}

HistoryType::~HistoryType()
{
}

//////////////////////////////

HistoryTypeNone::HistoryTypeNone()
{
}

bool HistoryTypeNone::isEnabled() const
{
    return false;
}

HistoryScroll* HistoryTypeNone::scroll(HistoryScroll* old) const
{
    delete old;
    return new HistoryScrollNone();
}

int HistoryTypeNone::maximumLineCount() const
{
    return 0;
}

//////////////////////////////

HistoryTypeBuffer::HistoryTypeBuffer(unsigned int nbLines)
    : m_nbLines(nbLines)
{
}

bool HistoryTypeBuffer::isEnabled() const
{
    return true;
}

int HistoryTypeBuffer::maximumLineCount() const
{
    return m_nbLines;
}

HistoryScroll* HistoryTypeBuffer::scroll(HistoryScroll* old) const
{
    if (old)
    {
        HistoryScrollBuffer* oldBuffer = dynamic_cast<HistoryScrollBuffer*>(old);
        if (oldBuffer)
        {
            oldBuffer->setMaxNbLines(m_nbLines);
            return oldBuffer;
        }

        HistoryScroll* newScroll = new HistoryScrollBuffer(m_nbLines);
        int lines = old->getLines();
        int startLine = 0;
        if (lines > (int) m_nbLines)
        {
            startLine = lines - m_nbLines;
        }

        Character line[LINE_SIZE];
        for (int i = startLine; i < lines; i++)
        {
            int size = old->getLineLen(i);
            if (size > LINE_SIZE)
            {
                Character* tmp_line = new Character[size];
                old->getCells(i, 0, size, tmp_line);
                newScroll->addCells(tmp_line, size);
                newScroll->addLine(old->isWrappedLine(i));
                delete[] tmp_line;
            }
            else
            {
                old->getCells(i, 0, size, line);
                newScroll->addCells(line, size);
                newScroll->addLine(old->isWrappedLine(i));
            }
        }
        delete old;
        return newScroll;
    }
    return new HistoryScrollBuffer(m_nbLines);
}
