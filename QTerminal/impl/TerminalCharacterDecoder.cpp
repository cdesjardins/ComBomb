/*
    This file is part of Konsole, an X terminal.

    Copyright (C) 2006 by Robert Knight <robertknight@gmail.com>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "TerminalCharacterDecoder.h"

// Qt
#include <QtCore/QTextStream>

//a standard set of colors using black text on a white background.
//defined in TerminalDisplay.cpp

static const ColorEntry s_baseColorTable[TABLE_COLORS] =
{
    // normal
    ColorEntry(QColor(0x00, 0x00, 0x00), 1, 0), ColorEntry(QColor(0xFF, 0xFF, 0xFF), 0, 0), // Dfore, Dback
    ColorEntry(QColor(0x00, 0x00, 0x00), 0, 0), ColorEntry(QColor(0xB2, 0x18, 0x18), 0, 0), // Black, Red
    ColorEntry(QColor(0x18, 0xB2, 0x18), 0, 0), ColorEntry(QColor(0xB2, 0x68, 0x18), 0, 0), // Green, Yellow
    ColorEntry(QColor(0x18, 0x18, 0xB2), 0, 0), ColorEntry(QColor(0xB2, 0x18, 0xB2), 0, 0), // Blue, Magenta
    ColorEntry(QColor(0x18, 0xB2, 0xB2), 0, 0), ColorEntry(QColor(0xB2, 0xB2, 0xB2), 0, 0), // Cyan, White
    // intensiv
    ColorEntry(QColor(0x00, 0x00, 0x00), 0, 1), ColorEntry(QColor(0xFF, 0xFF, 0xFF), 1, 0),
    ColorEntry(QColor(0x68, 0x68, 0x68), 0, 0), ColorEntry(QColor(0xFF, 0x54, 0x54), 0, 0),
    ColorEntry(QColor(0x54, 0xFF, 0x54), 0, 0), ColorEntry(QColor(0xFF, 0xFF, 0x54), 0, 0),
    ColorEntry(QColor(0x54, 0x54, 0xFF), 0, 0), ColorEntry(QColor(0xFF, 0x54, 0xFF), 0, 0),
    ColorEntry(QColor(0x54, 0xFF, 0xFF), 0, 0), ColorEntry(QColor(0xFF, 0xFF, 0xFF), 0, 0)
};

const ColorEntry* TerminalCharacterDecoder::getBaseColorTable()
{
    return s_baseColorTable;
}

PlainTextDecoder::PlainTextDecoder()
    : _output(0)
    , _includeTrailingWhitespace(true)
{
}

void PlainTextDecoder::setTrailingWhitespace(bool enable)
{
    _includeTrailingWhitespace = enable;
}

bool PlainTextDecoder::trailingWhitespace() const
{
    return _includeTrailingWhitespace;
}

void PlainTextDecoder::begin(QTextStream* output)
{
    _output = output;
}

void PlainTextDecoder::end()
{
    _output = 0;
}

void PlainTextDecoder::decodeLine(const std::vector<Character>::const_iterator& characters, int count, LineProperty)
{
    Q_ASSERT(_output);

    //TODO should we ignore or respect the LINE_WRAPPED line property?

    //note:  we build up a QString and send it to the text stream rather writing into the text
    //stream a character at a time because it is more efficient.
    //(since QTextStream always deals with QStrings internally anyway)
    QString plainText;
    plainText.reserve(count);

    int outputCount = count;

    // if inclusion of trailing whitespace is disabled then find the end of the
    // line
    if (!_includeTrailingWhitespace)
    {
        for (int i = count - 1; i >= 0; i--)
        {
            if (characters[i].getChar() != ' ')
            {
                break;
            }
            else
            {
                outputCount--;
            }
        }
    }

    for (int i = 0; i < outputCount; i++)
    {
        plainText.append(QChar(characters[i].getChar()));
    }

    *_output << plainText;
}

HTMLDecoder::HTMLDecoder() :
    _output(0)
    , _colorTable(s_baseColorTable)
    , _innerSpanOpen(false)
    , _lastRendition(DEFAULT_RENDITION)
{
}

void HTMLDecoder::begin(QTextStream* output)
{
    _output = output;

    QString text;

    //open monospace span
    openSpan(text, "font-family:monospace");

    *output << text;
}

void HTMLDecoder::end()
{
    Q_ASSERT(_output);

    QString text;

    closeSpan(text);

    *_output << text;

    _output = 0;
}

//TODO: Support for LineProperty (mainly double width , double height)
void HTMLDecoder::decodeLine(const std::vector<Character>::const_iterator& characters, int count, LineProperty)
{
    Q_ASSERT(_output);

    QString text;

    int spaceCount = 0;

    for (int i = 0; i < count; i++)
    {
        QChar ch(characters[i].getChar());

        //check if appearance of character is different from previous char
        if (characters[i].getRendition() != _lastRendition  ||
            characters[i].getForegroundColor() != _lastForeColor  ||
            characters[i].getBackgroundColor() != _lastBackColor)
        {
            if (_innerSpanOpen)
            {
                closeSpan(text);
            }

            _lastRendition = characters[i].getRendition();
            _lastForeColor = characters[i].getForegroundColor();
            _lastBackColor = characters[i].getBackgroundColor();

            //build up style string
            QString style;

            if (_lastRendition & RENDITION_BOLD ||
                (_colorTable && characters[i].isBold(_colorTable)))
            {
                style.append("font-weight:bold;");
            }

            if (_lastRendition & RENDITION_UNDERLINE)
            {
                style.append("font-decoration:underline;");
            }

            //colours - a colour table must have been defined first
            if (_colorTable)
            {
                style.append(QString("color:%1;").arg(_lastForeColor.color(_colorTable).name()));

                if (!characters[i].isTransparent(_colorTable))
                {
                    style.append(QString("background-color:%1;").arg(_lastBackColor.color(_colorTable).name()));
                }
            }

            //open the span with the current style
            openSpan(text, style);
            _innerSpanOpen = true;
        }

        //handle whitespace
        if (ch.isSpace())
        {
            spaceCount++;
        }
        else
        {
            spaceCount = 0;
        }

        //output current character
        if (spaceCount < 2)
        {
            //escape HTML tag characters and just display others as they are
            if (ch == '<')
            {
                text.append("&lt;");
            }
            else if (ch == '>')
            {
                text.append("&gt;");
            }
            else
            {
                text.append(ch);
            }
        }
        else
        {
            text.append("&nbsp;"); //HTML truncates multiple spaces, so use a space marker instead
        }
    }

    //close any remaining open inner spans
    if (_innerSpanOpen)
    {
        closeSpan(text);
    }

    //start new line
    text.append("<br>");

    *_output << text;
}

void HTMLDecoder::openSpan(QString& text, const QString& style)
{
    text.append(QString("<span style=\"%1\">").arg(style));
}

void HTMLDecoder::closeSpan(QString& text)
{
    text.append("</span>");
}

void HTMLDecoder::setColorTable(const ColorEntry* table)
{
    _colorTable = table;
}
