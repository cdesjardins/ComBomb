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
class HistoryType;

class HistoryScroll
{
public:
    HistoryScroll(HistoryType*);
    virtual ~HistoryScroll();

    virtual bool hasScroll();

    // access to history
    virtual int getLines() = 0;
    virtual int getLineLen(int lineno) = 0;
    virtual void getCells(int lineno, int colno, int count, Character res[]) = 0;
    virtual bool isWrappedLine(int lineno) = 0;

    // backward compatibility (obsolete)
    Character getCell(int lineno, int colno)
    {
        Character res; getCells(lineno, colno, 1, &res); return res;
    }

    // adding lines.
    virtual void addCells(const Character a[], int count) = 0;
    // convenience method - this is virtual so that subclasses can take advantage
    // of QVector's implicit copying
    virtual void addCellsVector(const QVector<Character>& cells)
    {
        addCells(cells.data(), cells.size());
    }

    virtual void addLine(bool previousWrapped = false) = 0;

    //
    // FIXME:  Passing around constant references to HistoryType instances
    // is very unsafe, because those references will no longer
    // be valid if the history scroll is deleted.
    //
    const HistoryType& getType()
    {
        return *m_histType;
    }

protected:
    HistoryType* m_histType;
};

//////////////////////////////////////////////////////////////////////
// Buffer-based history (limited to a fixed nb of lines)
//////////////////////////////////////////////////////////////////////
class HistoryScrollBuffer : public HistoryScroll
{
public:
    typedef QVector<Character> HistoryLine;

    HistoryScrollBuffer(unsigned int maxNbLines = 1000);
    virtual ~HistoryScrollBuffer();

    virtual int getLines();
    virtual int getLineLen(int lineno);
    virtual void getCells(int lineno, int colno, int count, Character res[]);
    virtual bool isWrappedLine(int lineno);

    virtual void addCells(const Character a[], int count);
    virtual void addCellsVector(const QVector<Character>& cells);
    virtual void addLine(bool previousWrapped = false);

    void setMaxNbLines(unsigned int nbLines);
    unsigned int maxNbLines()
    {
        return _maxLineCount;
    }

private:
    int bufferIndex(int lineNumber);

    HistoryLine* _historyBuffer;
    QBitArray _wrappedLine;
    int _maxLineCount;
    int _usedLines;
    int _head;
};

//////////////////////////////////////////////////////////////////////
// Nothing-based history (no history :-)
//////////////////////////////////////////////////////////////////////
class HistoryScrollNone : public HistoryScroll
{
public:
    HistoryScrollNone();
    virtual ~HistoryScrollNone();

    virtual bool hasScroll();

    virtual int getLines();
    virtual int getLineLen(int lineno);
    virtual void getCells(int lineno, int colno, int count, Character res[]);
    virtual bool isWrappedLine(int lineno);

    virtual void addCells(const Character a[], int count);
    virtual void addLine(bool previousWrapped = false);
};

//////////////////////////////////////////////////////////////////////
// History type
//////////////////////////////////////////////////////////////////////

class HistoryType
{
public:
    HistoryType();
    virtual ~HistoryType();

    /**
     * Returns true if the history is enabled ( can store lines of output )
     * or false otherwise.
     */
    virtual bool isEnabled()           const = 0;
    /**
     * Returns true if the history size is unlimited.
     */
    bool isUnlimited() const
    {
        return maximumLineCount() == 0;
    }

    /**
     * Returns the maximum number of lines which this history type
     * can store or 0 if the history can store an unlimited number of lines.
     */
    virtual int maximumLineCount()    const = 0;

    virtual HistoryScroll* scroll(HistoryScroll*) const = 0;
};

class HistoryTypeNone : public HistoryType
{
public:
    HistoryTypeNone();

    virtual bool isEnabled() const;
    virtual int maximumLineCount() const;

    virtual HistoryScroll* scroll(HistoryScroll*) const;
};

class HistoryTypeBuffer : public HistoryType
{
public:
    HistoryTypeBuffer(unsigned int nbLines);

    virtual bool isEnabled() const;
    virtual int maximumLineCount() const;

    virtual HistoryScroll* scroll(HistoryScroll*) const;

protected:
    unsigned int m_nbLines;
};

#endif // HISTORY_H
