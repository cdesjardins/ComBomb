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

#ifndef HISTORY_H
#define HISTORY_H

// Qt
#include <QtCore/QBitRef>
#include <QtCore/QHash>
#include <QtCore>

// Konsole
#include "Character.h"

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Abstract base class for file and buffer versions
//////////////////////////////////////////////////////////////////////

class HistoryScroll
{
public:
    HistoryScroll();
    virtual ~HistoryScroll();

    virtual bool hasScroll();

    // access to history
    virtual int getLines() = 0;
    virtual int getLineLen(int lineno) = 0;
    virtual void getCells(int lineno, int colno, int count, Character res[]) = 0;
    virtual bool isWrappedLine(int lineno) = 0;
    virtual void clearHistory() = 0;
    // backward compatibility (obsolete)
    Character getCell(int lineno, int colno)
    {
        Character res; getCells(lineno, colno, 1, &res); return res;
    }

    virtual void addCellsVector(const std::vector<Character>& cells) = 0;
    virtual void addLine(bool previousWrapped = false) = 0;

protected:
};

//////////////////////////////////////////////////////////////////////
// Buffer-based history (limited to a fixed nb of lines)
//////////////////////////////////////////////////////////////////////
class HistoryScrollBuffer : public HistoryScroll
{
public:
    HistoryScrollBuffer(unsigned int maxNbLines);
    virtual ~HistoryScrollBuffer();

    virtual int getLines();
    virtual int getLineLen(int lineno);
    virtual void getCells(int lineno, int colno, int count, Character res[]);
    virtual bool isWrappedLine(int lineno);
    virtual void clearHistory();

    //virtual void addCells(const Character a[], int count);
    virtual void addCellsVector(const std::vector<Character>& cells);
    virtual void addLine(bool previousWrapped = false);
protected:
    void growScrollback();

private:

    std::vector<std::vector<Character> >_historyBuffer;
    QBitArray _wrappedLine;
    int _maxLineCount;
    int _growCount;
    int _usedLines;
};

#endif // HISTORY_H
