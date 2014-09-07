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
#include "Screen.h"

// Standard
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

// Qt
#include <QtCore/QTextStream>
#include <QtCore/QDate>

// Konsole
#include "konsole_wcwidth.h"
#include "TerminalCharacterDecoder.h"

//FIXME: this is emulation specific. Use false for xterm, true for ANSI.
//FIXME: see if we can get this from terminfo.
#define BS_CLEARS false

//Macro to convert x,y position on screen to position within an image.
//
//Originally the image was stored as one large contiguous block of
//memory, so a position within the image could be represented as an
//offset from the beginning of the block.  For efficiency reasons this
//is no longer the case.
//Many internal parts of this class still use this representation for parameters and so on,
//notably moveImage() and clearImage().
//This macro converts from an X,Y position into an image offset.
#ifndef loc
#define loc(X, Y) ((Y)*_columns + (X))
#endif

Character Screen::_defaultChar = Character(' ',
                                          CharacterColor(COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR),
                                          CharacterColor(COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR),
                                          DEFAULT_RENDITION);

//#define REVERSE_WRAPPED_LINES  // for wrapped line debug

Screen::Screen(const boost::shared_ptr<HistoryScroll> &hist, int l, int c)
    : _lines(l),
    _columns(c),
    _screenLinesHead(0),
    _scrolledLines(0),
    _droppedLines(0),
    _hist(hist),
    _cursorX(0),
    _cursorY(0),
    _cursorRe(0),
    _topMargin(0),
    _bottomMargin(0),
    _tabstops(0),
    _selectionBegin(0),
    _selectionTopLeft(0),
    _selectionBottomRight(0),
    _columnMode(false),
    _effectiveCursorFg(CharacterColor()),
    _effectiveCursorBg(CharacterColor()),
    _effectiveCursorRe(0),
    _savedCursorX(0),
    _savedCursorY(0),
    _savedCursorRe(0),
    _lastPos(-1)
{
    _screenLines.resize(_lines + 1);
    _lineProperties.resize(_lines + 1);
    for (int i = 0; i < _lines + 1; i++)
    {
        _lineProperties[i] = LINE_DEFAULT;
    }

    initTabStops();
    clearSelection();
    reset();
}

/*! Destructor
*/

Screen::~Screen()
{
    delete[] _tabstops;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* Normalized                    Screen Operations                           */
/*                                                                           */
/* ------------------------------------------------------------------------- */

// Cursor Setting --------------------------------------------------------------

/*! \section Cursor

    The `cursor' is a location within the screen that is implicitely used in
    many operations. The operations within this section allow to manipulate
    the cursor explicitly and to obtain it's value.

    The position of the cursor is guarantied to be between (including) 0 and
    `columns-1' and `lines-1'.
*/

/*!
    Move the cursor up.

    The cursor will not be moved beyond the top margin.
*/

void Screen::cursorUp(int n)
//=CUU
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    int stop = _cursorY < _topMargin ? 0 : _topMargin;
    _cursorX = qMin(_columns - 1, _cursorX); // nowrap!
    _cursorY = qMax(stop, _cursorY - n);
}

/*!
    Move the cursor down.

    The cursor will not be moved beyond the bottom margin.
*/

void Screen::cursorDown(int n)
//=CUD
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    int stop = _cursorY > _bottomMargin ? _lines - 1 : _bottomMargin;
    _cursorX = qMin(_columns - 1, _cursorX); // nowrap!
    _cursorY = qMin(stop, _cursorY + n);
}

/*!
    Move the cursor left.

    The cursor will not move beyond the first column.
*/

void Screen::cursorLeft(int n)
//=CUB
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    _cursorX = qMin(_columns - 1, _cursorX); // nowrap!
    _cursorX = qMax(0, _cursorX - n);
}

/*!
    Move the cursor left.

    The cursor will not move beyond the rightmost column.
*/

void Screen::cursorRight(int n)
//=CUF
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    _cursorX = qMin(_columns - 1, _cursorX + n);
}

void Screen::setMargins(int top, int bot)
//=STBM
{
    if (top == 0)
    {
        top = 1;              // Default
    }
    if (bot == 0)
    {
        bot = _lines;          // Default
    }
    top = top - 1;            // Adjust to internal lineno
    bot = bot - 1;            // Adjust to internal lineno
    if (!(0 <= top && top < bot && bot < _lines))
    {
        //qDebug() << " setRegion(" << top << "," << bot << ") : bad range.";
        return;               // Default error action: ignore
    }
    _topMargin = top;
    _bottomMargin = bot;
    _cursorX = 0;
    _cursorY = getMode(MODE_Origin) ? top : 0;
}

int Screen::topMargin() const
{
    return _topMargin;
}

int Screen::bottomMargin() const
{
    return _bottomMargin;
}

void Screen::index()
//=IND
{
    if (_cursorY == _bottomMargin)
    {
        scrollUp(1);
    }
    else if (_cursorY < _lines - 1)
    {
        _cursorY += 1;
    }
}

void Screen::reverseIndex()
//=RI
{
    if (_cursorY == _topMargin)
    {
        scrollDown(_topMargin, 1);
    }
    else if (_cursorY > 0)
    {
        _cursorY -= 1;
    }
}

/*!
    Move the cursor to the begin of the next line.

    If cursor is on bottom margin, the region between the
    actual top and bottom margin is scrolled up.
*/

void Screen::NextLine()
//=NEL
{
    Return(); index();
}

void Screen::eraseChars(int n)
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    int p = qMax(0, qMin(_cursorX + n - 1, _columns - 1));
    clearImage(loc(_cursorX, _cursorY), loc(p, _cursorY), ' ');
}

void Screen::deleteChars(int n)
{
    Q_ASSERT(n >= 0);

    // always delete at least one char
    if (n == 0)
    {
        n = 1;
    }

    // if cursor is beyond the end of the line there is nothing to do
    if ((size_t)_cursorX >= _screenLines[screenLineIndex(_cursorY)].size())
    {
        return;
    }

    if ((size_t)_cursorX + n >= _screenLines[screenLineIndex(_cursorY)].size())
    {
        n = _screenLines[screenLineIndex(_cursorY)].size() - 1 - _cursorX;
    }

    Q_ASSERT(n >= 0);
    Q_ASSERT((size_t)_cursorX + n < _screenLines[screenLineIndex(_cursorY)].size());

    //_screenLines[_cursorY].remove(_cursorX, n);
    _screenLines[screenLineIndex(_cursorY)].erase(
            _screenLines[screenLineIndex(_cursorY)].begin() + _cursorX,
            _screenLines[screenLineIndex(_cursorY)].begin() + _cursorX + n);
}

void Screen::insertChars(int n)
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    if (_screenLines[screenLineIndex(_cursorY)].size() < (size_t)_cursorX)
    {
        _screenLines[screenLineIndex(_cursorY)].resize(_cursorX);
    }

    _screenLines[screenLineIndex(_cursorY)].insert(_screenLines[screenLineIndex(_cursorY)].begin() + _cursorX, n, ' ');

    if (_screenLines[screenLineIndex(_cursorY)].size() > (size_t)_columns)
    {
        _screenLines[screenLineIndex(_cursorY)].resize(_columns);
    }
}

void Screen::deleteLines(int n)
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    scrollUp(_cursorY, n);
}

/*! insert n lines at the cursor position.

    The cursor is not moved by the operation.
*/

void Screen::insertLines(int n)
{
    if (n == 0)
    {
        n = 1;       // Default
    }
    scrollDown(_cursorY, n);
}

// Mode Operations -----------------------------------------------------------

/*! Set a specific mode. */

void Screen::setMode(int m)
{
    _currParm.mode[m] = true;
    switch (m)
    {
        case MODE_Origin: _cursorX = 0; _cursorY = _topMargin; break; //FIXME: home
    }
}

/*! Reset a specific mode. */

void Screen::resetMode(int m)
{
    _currParm.mode[m] = false;
    switch (m)
    {
        case MODE_Origin: _cursorX = 0; _cursorY = 0; break; //FIXME: home
    }
}

/*! Save a specific mode. */

void Screen::saveMode(int m)
{
    _saveParm.mode[m] = _currParm.mode[m];
}

/*! Restore a specific mode. */

void Screen::restoreMode(int m)
{
    _currParm.mode[m] = _saveParm.mode[m];
}

bool Screen::getMode(int m) const
{
    return _currParm.mode[m];
}

void Screen::saveCursor()
{
    _savedCursorX     = _cursorX;
    _savedCursorY     = _cursorY;
    _savedCursorRe   = _cursorRe;
    _savedCursorFg   = _cursorFg;
    _savedCursorBg   = _cursorBg;
}

void Screen::restoreCursor()
{
    _cursorX     = qMin(_savedCursorX, _columns - 1);
    _cursorY     = qMin(_savedCursorY, _lines - 1);
    _cursorRe   = _savedCursorRe;
    _cursorFg   = _savedCursorFg;
    _cursorBg   = _savedCursorBg;
    effectiveRendition();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                             Screen Operations                             */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/*! Resize the screen image

    The topmost left position is maintained, while lower lines
    or right hand side columns might be removed or filled with
    spaces to fit the new size.

    The region setting is reset to the whole screen and the
    tab positions reinitialized.

    If the new image is narrower than the old image then text on lines
    which extends past the end of the new image is preserved so that it becomes
    visible again if the screen is later resized to make it larger.
*/

void Screen::resizeImage(int new_lines, int new_columns)
{
    if ((new_lines == _lines) && (new_columns == _columns))
    {
        return;
    }

    if (_cursorY > new_lines - 1)
    { // attempt to preserve focus and lines
        _bottomMargin = _lines - 1; //FIXME: margin lost
        for (int i = 0; i < _cursorY - (new_lines - 1); i++)
        {
            addHistLine(); scrollUp(0, 1);
        }
    }

    // create new screen lines
    _screenLines.resize(new_lines + 1);
    for (int i = _lines; (i > 0) && (i < new_lines + 1); i++)
    {
        _screenLines[i].resize(new_columns);
    }

    _lineProperties.resize(new_lines + 1);
    for (int i = _lines; (i > 0) && (i < new_lines + 1); i++)
    {
        _lineProperties[i] = LINE_DEFAULT;
    }

    clearSelection();

    _lines = new_lines;
    _columns = new_columns;
    _cursorX = qMin(_cursorX, _columns - 1);
    _cursorY = qMin(_cursorY, _lines - 1);

    // FIXME: try to keep values, evtl.
    _topMargin = 0;
    _bottomMargin = _lines - 1;
    initTabStops();
    clearSelection();
}

void Screen::setDefaultMargins()
{
    _topMargin = 0;
    _bottomMargin = _lines - 1;
}

/*
   Clarifying rendition here and in the display.

   currently, the display's color table is
     0       1       2 .. 9    10 .. 17
     dft_fg, dft_bg, dim 0..7, intensive 0..7

   _cursorFg, _cursorBg contain values 0..8;
   - 0    = default color
   - 1..8 = ansi specified color

   re_fg, re_bg contain values 0..17
   due to the TerminalDisplay's color table

   rendition attributes are

      attr           widget screen
      -------------- ------ ------
      RE_UNDERLINE     XX     XX    affects foreground only
      RE_BLINK         XX     XX    affects foreground only
      RE_BOLD          XX     XX    affects foreground only
      RE_REVERSE       --     XX
      RE_TRANSPARENT   XX     --    affects background only
      RE_INTENSIVE     XX     --    affects foreground only

   Note that RE_BOLD is used in both widget
   and screen rendition. Since xterm/vt102
   is to poor to distinguish between bold
   (which is a font attribute) and intensive
   (which is a color attribute), we translate
   this and RE_BOLD in falls eventually appart
   into RE_BOLD and RE_INTENSIVE.
*/

void Screen::reverseRendition(Character& p) const
{
    CharacterColor f = p._foregroundColor;
    CharacterColor b = p._backgroundColor;

    p._foregroundColor = b;
    p._backgroundColor = f;
}

void Screen::effectiveRendition()
// calculate rendition
{
    //copy "current rendition" straight into "effective rendition", which is then later copied directly
    //into the image[] array which holds the characters and their appearance properties.
    //- The old version below filtered out all attributes other than underline and blink at this stage,
    //so that they would not be copied into the image[] array and hence would not be visible by TerminalDisplay
    //which actually paints the screen using the information from the image[] array.
    //I don't know why it did this, but I'm fairly sure it was the wrong thing to do.  The net result
    //was that bold text wasn't printed in bold by Konsole.
    _effectiveCursorRe = _cursorRe;

    //OLD VERSION:
    //_effectiveCursorRe = _cursorRe & (RE_UNDERLINE | RE_BLINK);

    if (_cursorRe & RE_REVERSE)
    {
        _effectiveCursorFg = _cursorBg;
        _effectiveCursorBg = _cursorFg;
    }
    else
    {
        _effectiveCursorFg = _cursorFg;
        _effectiveCursorBg = _cursorBg;
    }

    if (_cursorRe & RE_BOLD)
    {
        _effectiveCursorFg.toggleIntensive();
    }
}

/*!
    returns the image.

    Get the size of the image by \sa getLines and \sa getColumns.

    NOTE that the image returned by this function must later be
    freed.

*/

void Screen::copyFromHistory(std::vector<Character>::iterator dest, int startLine, int count) const
{
    Q_ASSERT(startLine >= 0 && count > 0 && startLine + count <= _hist->getLines());

    for (int line = startLine; line < startLine + count; line++)
    {
        const int length = qMin(_columns, _hist->getLineLen(line));
        const int destLineOffset  = (line - startLine) * _columns;

        _hist->getCells(line, 0, length, dest + destLineOffset, _defaultChar);

        for (int column = length; column < _columns; column++)
        {
            dest[destLineOffset + column] = _defaultChar;
        }

        // invert selected text
        if (_selectionBegin != -1)
        {
            for (int column = 0; column < _columns; column++)
            {
                if (isSelected(column, line))
                {
                    reverseRendition(dest[destLineOffset + column]);
                }
            }
        }
    }
}

void Screen::copyFromScreen(std::vector<Character>::iterator dest, int startLine, int count) const
{
    Q_ASSERT(startLine >= 0 && count > 0 && startLine + count <= _lines);

    for (int line = startLine; line < (startLine + count); line++)
    {
        int srcLineStartIndex  = line * _columns;
        int destLineStartIndex = (line - startLine) * _columns;

        for (int column = 0; column < _columns; column++)
        {
            int srcIndex = srcLineStartIndex + column;
            int destIndex = destLineStartIndex + column;

            if ((size_t)(srcIndex % _columns) >= _screenLines[screenLineIndex(srcIndex / _columns)].size())
            {
                dest[destIndex] = _defaultChar;
            }
            else
            {
                dest[destIndex] = _screenLines[screenLineIndex(srcIndex / _columns)].at(srcIndex % _columns);
            }

            // invert selected text
            if (_selectionBegin != -1 && isSelected(column, line + _hist->getLines()))
            {
                reverseRendition(dest[destIndex]);
            }
        }
    }
}

void Screen::getImage(std::vector<Character> &dest, int size, int startLine, int endLine) const
{
    Q_ASSERT(startLine >= 0);
    Q_ASSERT(endLine >= startLine && endLine < _hist->getLines() + _lines);

    const int mergedLines = endLine - startLine + 1;

    Q_ASSERT(size >= mergedLines * _columns);
    Q_UNUSED(size);

    const int linesInHistoryBuffer = qBound(0, _hist->getLines() - startLine, mergedLines);
    const int linesInScreenBuffer = mergedLines - linesInHistoryBuffer;

    // copy lines from history buffer
    if (linesInHistoryBuffer > 0)
    {
        copyFromHistory(dest.begin(), startLine, linesInHistoryBuffer);
    }

    // copy lines from screen buffer
    if (linesInScreenBuffer > 0)
    {
        copyFromScreen(dest.begin() + linesInHistoryBuffer * _columns,
                       startLine + linesInHistoryBuffer - _hist->getLines(),
                       linesInScreenBuffer);
    }

    // invert display when in screen mode
    if (getMode(MODE_Screen))
    {
        for (int i = 0; i < mergedLines * _columns; i++)
        {
            reverseRendition(dest[i]); // for reverse display
        }
    }

    // mark the character at the current cursor position
    int cursorIndex = loc(_cursorX, _cursorY + linesInHistoryBuffer);
    if (getMode(MODE_Cursor) && cursorIndex < _columns * mergedLines)
    {
        dest[cursorIndex]._rendition |= RE_CURSOR;
    }
}

std::vector<LineProperty> Screen::getLineProperties(int startLine, int endLine) const
{
    Q_ASSERT(startLine >= 0);
    Q_ASSERT(endLine >= startLine && endLine < _hist->getLines() + _lines);

    const int mergedLines = endLine - startLine + 1;
    const int linesInHistory = qBound(0, _hist->getLines() - startLine, mergedLines);
    const int linesInScreen = mergedLines - linesInHistory;

    std::vector<LineProperty> result(mergedLines);
    int index = 0;

    // copy properties for lines in history
    for (int line = startLine; line < startLine + linesInHistory; line++)
    {
        //TODO Support for line properties other than wrapped lines
        if (_hist->isWrappedLine(line))
        {
            result[index] = (LineProperty)(result[index] | LINE_WRAPPED);
        }
        index++;
    }

    // copy properties for lines in screen buffer
    const int firstScreenLine = startLine + linesInHistory - _hist->getLines();
    for (int line = firstScreenLine; line < firstScreenLine + linesInScreen; line++)
    {
        result[index] = _lineProperties[line];
        index++;
    }

    return result;
}

/*!
*/

void Screen::reset(bool clearScreen)
{
    setMode(MODE_Wrap); saveMode(MODE_Wrap);      // wrap at end of margin
    resetMode(MODE_Origin); saveMode(MODE_Origin); // position refere to [1,1]
    resetMode(MODE_Insert); saveMode(MODE_Insert); // overstroke
    setMode(MODE_Cursor);                         // cursor visible
    resetMode(MODE_Screen);                       // screen not inverse
    resetMode(MODE_NewLine);

    _topMargin = 0;
    _bottomMargin = _lines - 1;

    setDefaultRendition();
    saveCursor();

    if (clearScreen)
    {
        clear();
    }
}

/*! Clear the entire screen and home the cursor.
*/

void Screen::clear()
{
    clearEntireScreen();
    home();
}

void Screen::BackSpace()
{
    _cursorX = qMin(_columns - 1, _cursorX); // nowrap!
    _cursorX = qMax(0, _cursorX - 1);

    if (_screenLines[screenLineIndex(_cursorY)].size() < (size_t)_cursorX + 1)
    {
        _screenLines[screenLineIndex(_cursorY)].resize(_cursorX + 1);
    }

    if (BS_CLEARS)
    {
        _screenLines[screenLineIndex(_cursorY)][_cursorX]._character = ' ';
    }
}

void Screen::Tabulate(int n)
{
    // note that TAB is a format effector (does not write ' ');
    if (n == 0)
    {
        n = 1;
    }
    while ((n > 0) && (_cursorX < _columns - 1))
    {
        cursorRight(1); while ((_cursorX < _columns - 1) && !_tabstops[_cursorX])
            cursorRight(1);
        n--;
    }
}

void Screen::backTabulate(int n)
{
    // note that TAB is a format effector (does not write ' ');
    if (n == 0)
    {
        n = 1;
    }
    while ((n > 0) && (_cursorX > 0))
    {
        cursorLeft(1); while ((_cursorX > 0) && !_tabstops[_cursorX])
            cursorLeft(1);
        n--;
    }
}

void Screen::clearTabStops()
{
    for (int i = 0; i < _columns; i++)
    {
        _tabstops[i] = false;
    }
}

void Screen::changeTabStop(bool set)
{
    if (_cursorX >= _columns)
    {
        return;
    }
    _tabstops[_cursorX] = set;
}

void Screen::initTabStops()
{
    delete[] _tabstops;
    _tabstops = new bool[_columns];

    // Arrg! The 1st tabstop has to be one longer than the other.
    // i.e. the kids start counting from 0 instead of 1.
    // Other programs might behave correctly. Be aware.
    for (int i = 0; i < _columns; i++)
    {
        _tabstops[i] = (i % 8 == 0 && i != 0);
    }
}

/*!
   This behaves either as IND (Screen::Index) or as NEL (Screen::NextLine)
   depending on the NewLine Mode (LNM). This mode also
   affects the key sequence returned for newline ([CR]LF).
*/

void Screen::NewLine()
{
    if (getMode(MODE_NewLine))
    {
        Return();
    }
    index();
}

/*! put c literally onto the screen at the current cursor position.

    VT100 uses the convention to produce an automatic newline (am)
    with the *first* character that would fall onto the next line (xenl).
*/

void Screen::checkSelection(int from, int to)
{
    if (_selectionBegin == -1)
    {
        return;
    }
    int scr_TL = loc(0, _hist->getLines());
    //Clear entire selection if it overlaps region [from, to]
    if ((_selectionBottomRight > (from + scr_TL)) && (_selectionTopLeft < (to + scr_TL)))
    {
        clearSelection();
    }
}

void Screen::ShowCharacter(unsigned short c)
{
    // Note that VT100 does wrapping BEFORE putting the character.
    // This has impact on the assumption of valid cursor positions.
    // We indicate the fact that a newline has to be triggered by
    // putting the cursor one right to the last column of the screen.

    int w = konsole_wcwidth(c);

    if (w <= 0)
    {
        return;
    }

    if (_cursorX + w > _columns)
    {
        if (getMode(MODE_Wrap))
        {
            _lineProperties[_cursorY] = (LineProperty)(_lineProperties[_cursorY] | LINE_WRAPPED);
            NextLine();
        }
        else
        {
            _cursorX = _columns - w;
        }
    }

    // ensure current line vector has enough elements
    int size = _screenLines[screenLineIndex(_cursorY)].size();
    if (size == 0 && _cursorY > 0)
    {
        _screenLines[screenLineIndex(_cursorY)].resize(qMax(_screenLines[screenLineIndex(_cursorY - 1)].size(), (size_t)_cursorX + w));
    }
    else
    {
        if (size < _cursorX + w)
        {
            _screenLines[screenLineIndex(_cursorY)].resize(_columns);
        }
    }

    if (getMode(MODE_Insert))
    {
        insertChars(w);
    }

    _lastPos = loc(_cursorX, _cursorY);

    // check if selection is still valid.
    checkSelection(_cursorX, _cursorY);

    Character& currentChar = _screenLines[screenLineIndex(_cursorY)][_cursorX];

    currentChar._character = c;
    currentChar._foregroundColor = _effectiveCursorFg;
    currentChar._backgroundColor = _effectiveCursorBg;
    currentChar._rendition = _effectiveCursorRe;

    int i = 0;
    int newCursorX = _cursorX + w--;
    while (w)
    {
        i++;

        if (_screenLines[screenLineIndex(_cursorY)].size() < (size_t)_cursorX + i + 1)
        {
            _screenLines[screenLineIndex(_cursorY)].resize(_cursorX + i + 1);
        }

        Character& ch = _screenLines[screenLineIndex(_cursorY)][_cursorX + i];
        ch._character = 0;
        ch._foregroundColor = _effectiveCursorFg;
        ch._backgroundColor = _effectiveCursorBg;
        ch._rendition = _effectiveCursorRe;

        w--;
    }
    _cursorX = newCursorX;
}

void Screen::compose(const QString& /*compose*/)
{
    Q_ASSERT(0 /*Not implemented yet*/);
}

int Screen::scrolledLines() const
{
    return _scrolledLines;
}

int Screen::droppedLines() const
{
    return _droppedLines;
}

void Screen::resetDroppedLines()
{
    _droppedLines = 0;
}

void Screen::resetScrolledLines()
{
    //kDebug() << "scrolled lines reset";

    _scrolledLines = 0;
}

// Region commands -------------------------------------------------------------

void Screen::scrollUp(int n)
{
    if (n == 0)
    {
        n = 1;        // Default
    }
    if (_topMargin == 0)
    {
        addHistLine();              // hist.history
    }
    scrollUp(_topMargin, n);
}

/*! scroll up n lines within current region.
    The n new lines are cleared.
*/

QRect Screen::lastScrolledRegion() const
{
    return _lastScrolledRegion;
}

void Screen::scrollUp(int from, int n)
{
    if (n <= 0 || from + n > _bottomMargin)
    {
        return;
    }

    _scrolledLines -= n;
    _lastScrolledRegion = QRect(0, _topMargin, _columns - 1, (_bottomMargin - _topMargin));

    //FIXME: make sure _topMargin _bottomMargin from n is in bounds.
    moveImage(loc(0, from), loc(0, from + n), loc(_columns - 1, _bottomMargin));
    clearImage(loc(0, _bottomMargin - n + 1), loc(_columns - 1, _bottomMargin), ' ');
}

void Screen::scrollDown(int n)
{
    if (n == 0)
    {
        n = 1;        // Default
    }
    scrollDown(_topMargin, n);
}

/*! scroll down n lines within current region.
    The n new lines are cleared.
    setRegion scrollUp
*/

void Screen::scrollDown(int from, int n)
{
    //kDebug() << "Screen::scrollDown( from: " << from << " , n: " << n << ")";

    _scrolledLines += n;

    //FIXME: make sure _topMargin, _bottomMargin, from, n is in bounds.
    if (n <= 0)
    {
        return;
    }
    if (from > _bottomMargin)
    {
        return;
    }
    if (from + n > _bottomMargin)
    {
        n = _bottomMargin - from;
    }
    moveImage(loc(0, from + n), loc(0, from), loc(_columns - 1, _bottomMargin - n));
    clearImage(loc(0, from), loc(_columns - 1, from + n - 1), ' ');
}

void Screen::setCursorYX(int y, int x)
{
    setCursorY(y); setCursorX(x);
}

void Screen::setCursorX(int x)
{
    if (x == 0)
    {
        x = 1;       // Default
    }
    x -= 1; // Adjust
    _cursorX = qMax(0, qMin(_columns - 1, x));
}

void Screen::setCursorY(int y)
{
    if (y == 0)
    {
        y = 1;       // Default
    }
    y -= 1; // Adjust
    _cursorY = qMax(0, qMin(_lines  - 1, y + (getMode(MODE_Origin) ? _topMargin : 0)));
}

void Screen::home()
{
    _cursorX = 0;
    _cursorY = 0;
}

void Screen::Return()
{
    _cursorX = 0;
}

int Screen::getCursorX() const
{
    return _cursorX;
}

int Screen::getCursorY() const
{
    return _cursorY;
}

// Erasing ---------------------------------------------------------------------

/*! \section Erasing

    This group of operations erase parts of the screen contents by filling
    it with spaces colored due to the current rendition settings.

    Althought the cursor position is involved in most of these operations,
    it is never modified by them.
*/

/*! fill screen between (including) `loca' (start) and `loce' (end) with spaces.

    This is an internal helper functions. The parameter types are internal
    addresses of within the screen image and make use of the way how the
    screen matrix is mapped to the image vector.
*/

void Screen::clearImage(int loca, int loce, char c)
{
    int scr_TL = loc(0, _hist->getLines());
    //FIXME: check positions

    //Clear entire selection if it overlaps region to be moved...
    if ((_selectionBottomRight > (loca + scr_TL)) && (_selectionTopLeft < (loce + scr_TL)))
    {
        clearSelection();
    }

    int topLine = loca / _columns;
    int bottomLine = loce / _columns;

    Character clearCh(c, _cursorFg, _cursorBg, DEFAULT_RENDITION);

    //if the character being used to clear the area is the same as the
    //default character, the affected lines can simply be shrunk.
    bool isDefaultCh = (clearCh == Character());

    for (int y = topLine; y <= bottomLine; y++)
    {
        _lineProperties[y] = 0;

        int endCol = (y == bottomLine) ? loce % _columns : _columns - 1;
        int startCol = (y == topLine) ? loca % _columns : 0;

        std::vector<Character>& line = _screenLines[screenLineIndex(y)];

        if (isDefaultCh && endCol == _columns - 1)
        {
            line.resize(startCol);
        }
        else
        {
            if (line.size() < (size_t)endCol + 1)
            {
                line.resize(endCol + 1);
            }

            Character* data = line.data();
            for (int i = startCol; i <= endCol; i++)
            {
                data[i] = clearCh;
            }
        }
    }
}

/*! move image between (including) sourceBegin and sourceEnd to dest.

    The dest sourceBegin and sourceEnd parameters can be generated using
    the loc(column,line) macro.

    NOTE:  moveImage() can only move whole lines.

    This is an internal helper functions. The parameter types are internal
    addresses of within the screen image and make use of the way how the
    screen matrix is mapped to the image vector.
*/

void Screen::moveImage(int dest, int sourceBegin, int sourceEnd)
{
    Q_ASSERT(sourceBegin <= sourceEnd);

    int lines = (sourceEnd - sourceBegin) / _columns;
    int destLine = dest / _columns;
    int sourceBeginLine = sourceBegin / _columns;

    //move screen image and line properties:
    //the source and destination areas of the image may overlap,
    //so it matters that we do the copy in the right order -
    //forwards if dest < sourceBegin or backwards otherwise.
    //(search the web for 'memmove implementation' for details)
    //qDebug("move image: %i %i %i", dest, lines, _screenLines.size());

    if (dest < sourceBegin)
    {
        _screenLinesHead++;
        for (int i = 0; i <= lines; i++)
        {
            //_screenLines[destLine + i] = _screenLines[sourceBeginLine + i];
            _lineProperties[destLine + i] = _lineProperties[sourceBeginLine + i];
        }
    }
    else
    {
        _screenLinesHead--;
        for (int i = lines; i >= 0; i--)
        {
            //_screenLines[destLine + i] = _screenLines[sourceBeginLine + i];
            _lineProperties[destLine + i] = _lineProperties[sourceBeginLine + i];
        }
    }

    if (_lastPos != -1)
    {
        int diff = dest - sourceBegin; // Scroll by this amount
        _lastPos += diff;
        if ((_lastPos < 0) || (_lastPos >= (lines * _columns)))
        {
            _lastPos = -1;
        }
    }

    // Adjust selection to follow scroll.
    if (_selectionBegin != -1)
    {
        bool beginIsTL = (_selectionBegin == _selectionTopLeft);
        int diff = dest - sourceBegin; // Scroll by this amount
        int scr_TL = loc(0, _hist->getLines());
        int srca = sourceBegin + scr_TL; // Translate index from screen to global
        int srce = sourceEnd + scr_TL; // Translate index from screen to global
        int desta = srca + diff;
        int deste = srce + diff;

        if ((_selectionTopLeft >= srca) && (_selectionTopLeft <= srce))
        {
            _selectionTopLeft += diff;
        }
        else if ((_selectionTopLeft >= desta) && (_selectionTopLeft <= deste))
        {
            _selectionBottomRight = -1; // Clear selection (see below)
        }
        if ((_selectionBottomRight >= srca) && (_selectionBottomRight <= srce))
        {
            _selectionBottomRight += diff;
        }
        else if ((_selectionBottomRight >= desta) && (_selectionBottomRight <= deste))
        {
            _selectionBottomRight = -1; // Clear selection (see below)
        }
        if (_selectionBottomRight < 0)
        {
            clearSelection();
        }
        else
        {
            if (_selectionTopLeft < 0)
            {
                _selectionTopLeft = 0;
            }
        }

        if (beginIsTL)
        {
            _selectionBegin = _selectionTopLeft;
        }
        else
        {
            _selectionBegin = _selectionBottomRight;
        }
    }
}

void Screen::clearToEndOfScreen()
{
    clearImage(loc(_cursorX, _cursorY), loc(_columns - 1, _lines - 1), ' ');
}

void Screen::clearToBeginOfScreen()
{
    clearImage(loc(0, 0), loc(_cursorX, _cursorY), ' ');
}

void Screen::clearEntireScreen()
{
    clearImage(loc(0, 0), loc(_columns - 1, _lines - 1), ' ');
}

/*! fill screen with 'E'
    This is to aid screen alignment
*/

void Screen::helpAlign()
{
    clearImage(loc(0, 0), loc(_columns - 1, _lines - 1), 'E');
}

void Screen::clearToEndOfLine()
{
    clearImage(loc(_cursorX, _cursorY), loc(_columns - 1, _cursorY), ' ');
}

void Screen::clearToBeginOfLine()
{
    clearImage(loc(0, _cursorY), loc(_cursorX, _cursorY), ' ');
}

void Screen::clearEntireLine()
{
    clearImage(loc(0, _cursorY), loc(_columns - 1, _cursorY), ' ');
}

void Screen::setRendition(int re)
{
    _cursorRe |= re;
    effectiveRendition();
}

void Screen::resetRendition(int re)
{
    _cursorRe &= ~re;
    effectiveRendition();
}

void Screen::setDefaultRendition()
{
    setForeColor(COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR);
    setBackColor(COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR);
    _cursorRe   = DEFAULT_RENDITION;
    effectiveRendition();
}

void Screen::setForeColor(int space, int color)
{
    _cursorFg = CharacterColor(space, color);

    if (_cursorFg.isValid())
    {
        effectiveRendition();
    }
    else
    {
        setForeColor(COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR);
    }
}

void Screen::setBackColor(int space, int color)
{
    _cursorBg = CharacterColor(space, color);

    if (_cursorBg.isValid())
    {
        effectiveRendition();
    }
    else
    {
        setBackColor(COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR);
    }
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                            Marking & Selection                            */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void Screen::clearSelection()
{
    _selectionBottomRight = -1;
    _selectionTopLeft = -1;
    _selectionBegin = -1;
}

void Screen::getSelectionStart(int& column, int& line)
{
    if (_selectionTopLeft != -1)
    {
        column = _selectionTopLeft % _columns;
        line = _selectionTopLeft / _columns;
    }
    else
    {
        column = _cursorX + getHistLines();
        line = _cursorY + getHistLines();
    }
}

void Screen::getSelectionEnd(int& column, int& line)
{
    if (_selectionBottomRight != -1)
    {
        column = _selectionBottomRight % _columns;
        line = _selectionBottomRight / _columns;
    }
    else
    {
        column = _cursorX + getHistLines();
        line = _cursorY + getHistLines();
    }
}

void Screen::setSelectionStart(/*const ScreenCursor& viewCursor ,*/ const int x, const int y, const bool mode)
{
//  kDebug(1211) << "setSelBeginXY(" << x << "," << y << ")";
    _selectionBegin = loc(x, y); //+histCursor) ;

    /* FIXME, HACK to correct for x too far to the right... */
    if (x == _columns)
    {
        _selectionBegin--;
    }

    _selectionBottomRight = _selectionBegin;
    _selectionTopLeft = _selectionBegin;
    _columnMode = mode;
}

void Screen::setSelectionEnd(const int x, const int y)
{
//  kDebug(1211) << "setSelExtentXY(" << x << "," << y << ")";
    if (_selectionBegin == -1)
    {
        return;
    }
    int l =  loc(x, y); // + histCursor);

    if (l < _selectionBegin)
    {
        _selectionTopLeft = l;
        _selectionBottomRight = _selectionBegin;
    }
    else
    {
        /* FIXME, HACK to correct for x too far to the right... */
        if (x == _columns)
        {
            l--;
        }

        _selectionTopLeft = _selectionBegin;
        _selectionBottomRight = l;
    }
}

bool Screen::isSelected(const int x, const int y) const
{
    if (_columnMode)
    {
        int sel_Left, sel_Right;
        if (_selectionTopLeft % _columns < _selectionBottomRight % _columns)
        {
            sel_Left = _selectionTopLeft; sel_Right = _selectionBottomRight;
        }
        else
        {
            sel_Left = _selectionBottomRight; sel_Right = _selectionTopLeft;
        }
        return (x >= sel_Left % _columns) && (x <= sel_Right % _columns) &&
               (y >= _selectionTopLeft / _columns) && (y <= _selectionBottomRight / _columns);
    }
    else
    {
        //int pos = loc(x,y+histCursor);
        int pos = loc(x, y);
        return (pos >= _selectionTopLeft && pos <= _selectionBottomRight);
    }
}

QString Screen::selectedText(bool preserveLineBreaks)
{
    QString result;
    QTextStream stream(&result, QIODevice::ReadWrite);

    PlainTextDecoder decoder;
    decoder.begin(&stream);
    writeSelectionToStream(&decoder, preserveLineBreaks);
    decoder.end();

    return result;
}

bool Screen::isSelectionValid() const
{
    return (_selectionTopLeft >= 0 && _selectionBottomRight >= 0);
}

void Screen::writeSelectionToStream(TerminalCharacterDecoder* decoder,
                                    bool preserveLineBreaks)
{
    // do nothing if selection is invalid
    if (!isSelectionValid())
    {
        return;
    }

    int top = _selectionTopLeft / _columns;
    int left = _selectionTopLeft % _columns;

    int bottom = _selectionBottomRight / _columns;
    int right = _selectionBottomRight % _columns;

    Q_ASSERT(top >= 0 && left >= 0 && bottom >= 0 && right >= 0);

    for (int y = top; y <= bottom; y++)
    {
        int start = 0;
        if (y == top || _columnMode)
        {
            start = left;
        }

        int count = -1;
        if (y == bottom || _columnMode)
        {
            count = right - start + 1;
        }

        const bool appendNewLine = (y != bottom) || ((top == bottom) && (right == (_columns - 1)));
        copyLineToStream(y,
                         start,
                         count,
                         decoder,
                         appendNewLine,
                         preserveLineBreaks);
    }
}

void Screen::copyLineToStream(int line,
                              int start,
                              int count,
                              TerminalCharacterDecoder* decoder,
                              bool appendNewLine,
                              bool preserveLineBreaks)
{
    //buffer to hold characters for decoding
    //the buffer is static to avoid initialising every
    //element on each call to copyLineToStream
    //(which is unnecessary since all elements will be overwritten anyway)
    static const int MAX_CHARS = 1024;
    static std::vector<Character> characterBuffer;
    characterBuffer.resize(MAX_CHARS);

    assert(count < MAX_CHARS);

    LineProperty currentLineProperties = 0;

    //determine if the line is in the history buffer or the screen image
    if (line < _hist->getLines())
    {
        const int lineLength = _hist->getLineLen(line);

        // ensure that start position is before end of line
        start = qMin(start, qMax(0, lineLength - 1));

        //retrieve line from history buffer
        if (count == -1)
        {
            count = lineLength - start;
        }
        else
        {
            count = qMin(start + count, lineLength) - start;
        }

        // safety checks
        assert(start >= 0);
        assert(count >= 0);
        assert((start + count) <= _hist->getLineLen(line));

        _hist->getCells(line, start, count, characterBuffer.begin(), _defaultChar);

        if (_hist->isWrappedLine(line))
        {
            currentLineProperties |= LINE_WRAPPED;
        }
    }
    else
    {
        if (count == -1)
        {
            count = _columns - start;
        }

        assert(count >= 0);

        const int screenLine = line - _hist->getLines();

        std::vector<Character>::iterator data = _screenLines[screenLineIndex(screenLine)].begin();
        int length = _screenLines[screenLineIndex(screenLine)].size();

        //retrieve line from screen image
        for (int i = start; i < qMin(start + count, length); i++)
        {
            characterBuffer[i - start] = data[i];
        }

        // count cannot be any greater than length
        count = qBound(0, count, length - start);

        Q_ASSERT(screenLine < _lineProperties.count());
        currentLineProperties |= _lineProperties[screenLine];
    }

    //do not decode trailing whitespace characters
    for (int i = count - 1; i >= 0; i--)
    {
        if (QChar(characterBuffer[i]._character).isSpace())
        {
            count--;
        }
        else
        {
            break;
        }
    }

    // add new line character at end
    const bool omitLineBreak = (currentLineProperties & LINE_WRAPPED) ||
                               !preserveLineBreaks;

    if (!omitLineBreak && appendNewLine && (count + 1 < MAX_CHARS))
    {
        characterBuffer[count] = '\n';
        count++;
    }

    //decode line and write to text stream
    decoder->decodeLine(characterBuffer.begin(),
                        count, currentLineProperties);
}

QString Screen::getHistoryLine(int no)
{
    _selectionBegin = loc(0, no);
    _selectionTopLeft = _selectionBegin;
    _selectionBottomRight = loc(_columns - 1, no);
    return selectedText(false);
}

void Screen::addHistLine()
{
    // add line to history buffer
    // we have to take care about scrolling, too...

    if (hasScroll())
    {
        int oldHistLines = _hist->getLines();

        _hist->addCellsVector(_screenLines[screenLineIndex(0)], _lineProperties[0] & LINE_WRAPPED);

        int newHistLines = _hist->getLines();

        bool beginIsTL = (_selectionBegin == _selectionTopLeft);

        // If the history is full, increment the count
        // of dropped lines
        if (newHistLines == oldHistLines)
        {
            _droppedLines++;
        }

        // Adjust selection for the new point of reference
        if (newHistLines > oldHistLines)
        {
            if (_selectionBegin != -1)
            {
                _selectionTopLeft += _columns;
                _selectionBottomRight += _columns;
            }
        }

        if (_selectionBegin != -1)
        {
            // Scroll selection in history up
            int top_BR = loc(0, 1 + newHistLines);

            if (_selectionTopLeft < top_BR)
            {
                _selectionTopLeft -= _columns;
            }

            if (_selectionBottomRight < top_BR)
            {
                _selectionBottomRight -= _columns;
            }

            if (_selectionBottomRight < 0)
            {
                clearSelection();
            }
            else
            {
                if (_selectionTopLeft < 0)
                {
                    _selectionTopLeft = 0;
                }
            }

            if (beginIsTL)
            {
                _selectionBegin = _selectionTopLeft;
            }
            else
            {
                _selectionBegin = _selectionBottomRight;
            }
        }
    }
}

int Screen::getHistLines()
{
    return _hist->getLines();
}

void Screen::clearHistory()
{
    clearSelection();
    _hist->clearHistory();
}

bool Screen::hasScroll()
{
    return _hist->hasScroll();
}

void Screen::setLineProperty(LineProperty property, bool enable)
{
    if (enable)
    {
        _lineProperties[_cursorY] = (LineProperty)(_lineProperties[_cursorY] | property);
    }
    else
    {
        _lineProperties[_cursorY] = (LineProperty)(_lineProperties[_cursorY] & ~property);
    }
}

void Screen::fillWithDefaultChar(std::vector<Character>::iterator dest, int count)
{
    for (int i = 0; i < count; i++)
    {
        dest[i] = _defaultChar;
    }
}

