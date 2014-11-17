/*
    Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>

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
#include "ScreenWindow.h"

// Qt
#include <QtCore>

// Konsole
#include "Screen.h"

ScreenWindow::ScreenWindow(QObject* parent)
    : QObject(parent)
    , _windowBufferSize(0)
    , _bufferNeedsUpdate(true)
    , _windowLines(1)
    , _currentLine(0)
    , _trackOutput(true)
    , _scrollCount(0)
    , _searchLineForward(0)
    , _searchLineBackward(0)
    , _searchColForward(0)
    , _searchColBackward(-1)
{
}

ScreenWindow::~ScreenWindow()
{
}

void ScreenWindow::setScreen(Screen* screen)
{
    Q_ASSERT(screen);

    _screen = screen;
}

Screen* ScreenWindow::screen() const
{
    return _screen;
}

std::vector<Character> &ScreenWindow::getImage()
{
    // reallocate internal buffer if the window size has changed
    int size = windowLines() * windowColumns();
    if (_windowBufferSize != size)
    {
        _windowBufferSize = size;
        _windowBuffer.resize(size);
        _bufferNeedsUpdate = true;
    }

    if (!_bufferNeedsUpdate)
    {
        return _windowBuffer;
    }

    _screen->getImage(_windowBuffer, size,
                      currentLine(), endWindowLine());

    // this window may look beyond the end of the screen, in which
    // case there will be an unused area which needs to be filled
    // with blank characters
    fillUnusedArea();

    _bufferNeedsUpdate = false;
    return _windowBuffer;
}

void ScreenWindow::fillUnusedArea()
{
    int screenEndLine = _screen->getHistLines() + _screen->getLines() - 1;
    int windowEndLine = currentLine() + windowLines() - 1;

    int unusedLines = windowEndLine - screenEndLine;
    int charsToFill = unusedLines * windowColumns();

    if (charsToFill > 0)
    {
        Screen::fillWithDefaultChar((_windowBuffer.begin() + (_windowBufferSize - charsToFill)), charsToFill);
    }
}

// return the index of the line at the end of this window, or if this window
// goes beyond the end of the screen, the index of the line at the end
// of the screen.
//
// when passing a line number to a Screen method, the line number should
// never be more than endWindowLine()
//
int ScreenWindow::endWindowLine() const
{
    return qMin(currentLine() + windowLines() - 1,
                lineCount() - 1);
}

std::vector<LineProperty> ScreenWindow::getLineProperties()
{
    std::vector<LineProperty> result = _screen->getLineProperties(currentLine(), endWindowLine());

    if (result.size() != (size_t)windowLines())
    {
        result.resize(windowLines());
    }

    return result;
}

QString ScreenWindow::getSelectedText() const
{
    return _screen->getSelectedText();
}

void ScreenWindow::getSelectionStart(int& column, int& line)
{
    _screen->getSelectionStart(column, line);
}

void ScreenWindow::getSelectionEnd(int& column, int& line)
{
    _screen->getSelectionEnd(column, line);
}

void ScreenWindow::setSelectionAll()
{
    _screen->setSelectionStart(0, 0);
    _screen->setSelectionEnd(windowColumns(), lineCount());

    _bufferNeedsUpdate = true;
    emit selectionChanged();
}

void ScreenWindow::setSelectionStart(int column, int line)
{
    _screen->setSelectionStart(column, qMin(line + currentLine(), endWindowLine()));

    _bufferNeedsUpdate = true;
    emit selectionChanged();
}

void ScreenWindow::setSelectionEnd(int column, int line)
{
    _screen->setSelectionEnd(column, qMin(line + currentLine(), endWindowLine()));

    _bufferNeedsUpdate = true;
    emit selectionChanged();
}

int ScreenWindow::setSelectionFind(const int column, const int line, const int length)
{
    int ret = _screen->setSelectionFind(column, line, length);

    _bufferNeedsUpdate = true;
    emit selectionChanged();
    return ret;
}

void ScreenWindow::resetSearchPosition(const int searchCol)
{
    _searchLineForward = 0;
    _searchLineBackward = 0;
    _searchColForward = searchCol;
    _searchColBackward = searchCol - 1;
}

void ScreenWindow::setSearchPosition(const int searchCol, const int searchLine, const int length, const bool searchUp)
{
    int curLine = lineCount() - searchLine - 1;
    if (searchUp == false)
    {
        _searchLineForward = searchLine;
        _searchLineBackward = curLine;
    }
    else
    {
        _searchLineForward = curLine;
        _searchLineBackward = searchLine;
    }
    _searchColForward = searchCol + 1;
    _searchColBackward = -(length - searchCol) - 1;
}

bool ScreenWindow::findText(const QString& searchStr, const bool caseSensitive, const bool searchUp, const bool cont)
{
    bool ret = false;
    long scrollToLine = -1;
    if (cont == false)
    {
        resetSearchPosition(0);
    }
    long line = (searchUp == true) ? _searchLineBackward : _searchLineForward;
    while (line < lineCount())
    {
        QString str;
        long curLine = lineCount() - line - 1;
        if (searchUp == false)
        {
            curLine = line;
        }
        long numLines = _screen->writeLineToString(&curLine, str);
        if (numLines == 0)
        {
            break;
        }
        else
        {
            int index;
            if (searchUp == true)
            {
                index = str.lastIndexOf(searchStr, _searchColBackward,
                                        (caseSensitive == true) ? Qt::CaseSensitive : Qt::CaseInsensitive);
            }
            else
            {
                index = str.indexOf(searchStr, _searchColForward,
                                    (caseSensitive == true) ? Qt::CaseSensitive : Qt::CaseInsensitive);
            }
            if (index >= 0)
            {
                scrollToLine = setSelectionFind(index, curLine, searchStr.length());
                setSearchPosition(index, line, str.length(), searchUp);
                break;
            }
            line += numLines;
        }
        if (searchUp == true)
        {
            _searchColBackward = -1;
        }
        else
        {
            _searchColForward = 0;
        }
    }

    if (scrollToLine == -1)
    {
        resetSearchPosition(0);
    }
    else
    {
        ret = true;
        scrollTo(scrollToLine);
        setTrackOutput(atEndOfOutput());
    }

    return ret;
}

bool ScreenWindow::findTextHighlighted(QString* searchStr, const bool caseSensitive)
{
    bool ret = false;
    *searchStr = getSelectedText();
    if (searchStr->length() > 0)
    {
        int column = 0;
        int line = 0;
        getSelectionStart(column, line);
        setSearchPosition(column + 1, line, searchStr->length(), false);
        ret = findText(*searchStr, caseSensitive, false, true);
        if (ret == false)
        {
            ret = findText(*searchStr, caseSensitive, false, false);
        }
    }
    return ret;
}

bool ScreenWindow::isSelected(int column, int line)
{
    return _screen->isSelected(column, qMin(line + currentLine(), endWindowLine()));
}

void ScreenWindow::clearSelection()
{
    _screen->clearSelection();

    emit selectionChanged();
}

void ScreenWindow::setWindowLines(int lines)
{
    Q_ASSERT(lines > 0);
    _windowLines = lines;
}

int ScreenWindow::windowLines() const
{
    return _windowLines;
}

int ScreenWindow::windowColumns() const
{
    return _screen->getColumns();
}

int ScreenWindow::lineCount() const
{
    return _screen->getHistLines() + _screen->getLines();
}

int ScreenWindow::columnCount() const
{
    return _screen->getColumns();
}

QPoint ScreenWindow::cursorPosition() const
{
    QPoint position;

    position.setX(_screen->getCursorX());
    position.setY(_screen->getCursorY());

    return position;
}

int ScreenWindow::currentLine() const
{
    return qBound(0, _currentLine, lineCount() - windowLines());
}

void ScreenWindow::scrollBy(RelativeScrollMode mode, int amount)
{
    if (mode == ScrollLines)
    {
        scrollTo(currentLine() + amount);
    }
    else if (mode == ScrollPages)
    {
        scrollTo(currentLine() + amount * (windowLines() / 2));
    }
}

bool ScreenWindow::atEndOfOutput() const
{
    return currentLine() == (lineCount() - windowLines());
}

void ScreenWindow::scrollTo(int line)
{
    int maxCurrentLineNumber = lineCount() - windowLines();
    line = qBound(0, line, maxCurrentLineNumber);

    const int delta = line - _currentLine;
    _currentLine = line;

    // keep track of number of lines scrolled by,
    // this can be reset by calling resetScrollCount()
    _scrollCount += delta;

    _bufferNeedsUpdate = true;

    emit scrolled(_currentLine);
}

void ScreenWindow::setTrackOutput(bool trackOutput)
{
    _trackOutput = trackOutput;
}

bool ScreenWindow::trackOutput() const
{
    return _trackOutput;
}

int ScreenWindow::scrollCount() const
{
    return _scrollCount;
}

void ScreenWindow::resetScrollCount()
{
    _scrollCount = 0;
}

QRect ScreenWindow::scrollRegion() const
{
    bool equalToScreenSize = windowLines() == _screen->getLines();

    if (atEndOfOutput() && equalToScreenSize)
    {
        return _screen->lastScrolledRegion();
    }
    else
    {
        return QRect(0, 0, windowColumns(), windowLines());
    }
}

void ScreenWindow::notifyOutputChanged()
{
    // move window to the bottom of the screen and update scroll count
    // if this window is currently tracking the bottom of the screen
    if (_trackOutput)
    {
        _scrollCount -= _screen->scrolledLines();
        _currentLine = qMax(0, _screen->getHistLines() - (windowLines() - _screen->getLines()));
    }
    else
    {
        // if the history is not unlimited then it may
        // have run out of space and dropped the oldest
        // lines of output - in this case the screen
        // window's current line number will need to
        // be adjusted - otherwise the output will scroll
        _currentLine = qMax(0, _currentLine -
                            _screen->droppedLines());

        // ensure that the screen window's current position does
        // not go beyond the bottom of the screen
        _currentLine = qMin(_currentLine, _screen->getHistLines());
    }

    _bufferNeedsUpdate = true;

    emit outputChanged();
}

