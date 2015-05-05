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

// Konsole
#include "Character.h"
#include <boost/circular_buffer.hpp>

//
// Buffer-based history
//
class History
{
public:
    History() = delete;
    History(const History&) = delete;
    History(size_t histSize);
    virtual ~History();

    virtual int getLines();
    virtual int getLineLen(size_t lineno);
    virtual void getCells(size_t lineno, int colno, int count, std::vector<Character>::iterator res, Character defaultChar);
    virtual bool isWrappedLine(size_t lineno);
    virtual void clearHistory();
    // Returns true if the new histSize is smaller than the previous histSize
    virtual bool resizeHistory(size_t histSize);
    virtual void addCellsVector(const std::vector<Character>& cells, bool previousWrapped);
protected:

private:
    boost::circular_buffer<std::vector<Character> > _historyBuffer;

    QBitArray _wrappedLine;
};

#endif // HISTORY_H
