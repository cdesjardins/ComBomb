/*
    This file is part of Konsole, KDE's terminal.

    Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>
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

#ifndef CHARACTER_H
#define CHARACTER_H

// Qt
#include <QtCore/QHash>

// Local
#include "CharacterColor.h"

typedef unsigned char LineProperty;

static const int LINE_DEFAULT       = 0;
static const int LINE_WRAPPED       = (1 << 0);
static const int LINE_DOUBLEWIDTH   = (1 << 1);
static const int LINE_DOUBLEHEIGHT  = (1 << 2);

#define DEFAULT_RENDITION  0
#define RENDITION_BOLD            (1 << 0)
#define RENDITION_BLINK           (1 << 1)
#define RENDITION_UNDERLINE       (1 << 2)
#define RENDITION_REVERSE         (1 << 3)
#define RENDITION_CURSOR          (1 << 4)
#define RENDITION_EXTENDED_CHAR   (1 << 5)
#define RENDITION_RENDER          (1 << 6)

/**
 * A single character in the terminal which consists of a unicode character
 * value, foreground and background colors and a set of rendition attributes
 * which specify how it should be drawn.
 */

class Character
{
public:
    /**
     * Constructs a new character.
     *
     * @param c The unicode character value of this character.
     * @param f The foreground color used to draw the character.
     * @param b The color used to draw the character's background.
     * @param r A set of rendition flags which specify how this character is to be drawn.
     */
    inline Character(quint16 c = ' ',
                     CharacterColor f = CharacterColor(COLOR_SPACE_DEFAULT, CharacterColor::getDefaultForeColor()),
                     CharacterColor b = CharacterColor(COLOR_SPACE_DEFAULT, CharacterColor::getDefaultBackColor()),
                     quint8 r = DEFAULT_RENDITION)
        : _character(c),
        _rendition(r),
        _foregroundColor(f),
        _backgroundColor(b)
    {
    }

    void setProperties(quint16 c, CharacterColor f, CharacterColor b, quint8 r)
    {
        _character = c;
        _rendition = r;
        _foregroundColor = f;
        _backgroundColor = b;
    }

    quint16 getChar() const
    {
        return _character;
    }

    quint16 getCharSequence() const
    {
        return _charSequence;
    }

    void setChar(quint16 ch)
    {
        _character = ch;
    }

    quint8 getRendition() const
    {
        return _rendition;
    }

    void setRendition(quint8 rendition)
    {
        _rendition = rendition;
    }

    void updateRendition(quint8 rendition)
    {
        _rendition |= rendition;
    }

    CharacterColor getForegroundColor() const
    {
        return _foregroundColor;
    }

    void setForegroundColor(const CharacterColor& foregroundColor)
    {
        _foregroundColor = foregroundColor;
    }

    CharacterColor getBackgroundColor() const
    {
        return _backgroundColor;
    }

    void setBackgroundColor(const CharacterColor& backgroundColor)
    {
        _backgroundColor = backgroundColor;
    }

    /**
     * Returns true if this character has a transparent background when
     * it is drawn with the specified @p palette.
     */
    bool isTransparent(const ColorEntry* palette) const;
    /**
     * Returns true if this character should always be drawn in bold when
     * it is drawn with the specified @p palette, independent of whether
     * or not the character has the RE_BOLD rendition flag.
     */
    bool isBold(const ColorEntry* base) const;

    /**
     * Compares two characters and returns true if they have the same unicode character value,
     * rendition and colors.
     */
    friend bool operator ==(const Character& a, const Character& b);
    /**
     * Compares two characters and returns true if they have different unicode character values,
     * renditions or colors.
     */
    friend bool operator !=(const Character& a, const Character& b);
private:
    union
    {
        /** The unicode character value for this character. */
        quint16 _character;
        /**
         * Experimental addition which allows a single Character instance to contain more than
         * one unicode character.
         *
         * charSequence is a hash code which can be used to look up the unicode
         * character sequence in the ExtendedCharTable used to create the sequence.
         */
        quint16 _charSequence;
    };

    /** A combination of RENDITION flags which specify options for drawing the character. */
    quint8 _rendition;

    /** The foreground color used to draw this character. */
    CharacterColor _foregroundColor;

    /** The color used to draw this character's background. */
    CharacterColor _backgroundColor;
};

inline bool operator ==(const Character& a, const Character& b)
{
    return a._character == b._character &&
           a._rendition == b._rendition &&
           a._foregroundColor == b._foregroundColor &&
           a._backgroundColor == b._backgroundColor;
}

inline bool operator !=(const Character& a, const Character& b)
{
    return a._character != b._character ||
           a._rendition != b._rendition ||
           a._foregroundColor != b._foregroundColor ||
           a._backgroundColor != b._backgroundColor;
}

inline bool Character::isTransparent(const ColorEntry* base) const
{
    return ((_backgroundColor._colorSpace == COLOR_SPACE_DEFAULT) &&
            base[_backgroundColor._u + 0 + (_backgroundColor._v ? BASE_COLORS : 0)].transparent)
           || ((_backgroundColor._colorSpace == COLOR_SPACE_SYSTEM) &&
               base[_backgroundColor._u + 2 + (_backgroundColor._v ? BASE_COLORS : 0)].transparent);
}

inline bool Character::isBold(const ColorEntry* base) const
{
    return ((_backgroundColor._colorSpace == COLOR_SPACE_DEFAULT) &&
            base[_backgroundColor._u + 0 + (_backgroundColor._v ? BASE_COLORS : 0)].bold)
           || ((_backgroundColor._colorSpace == COLOR_SPACE_SYSTEM) &&
               base[_backgroundColor._u + 2 + (_backgroundColor._v ? BASE_COLORS : 0)].bold);
}

extern unsigned short vt100_graphics[32];

/**
 * A table which stores sequences of unicode characters, referenced
 * by hash keys.  The hash key itself is the same size as a unicode
 * character ( ushort ) so that it can occupy the same space in
 * a structure.
 */
class ExtendedCharTable
{
public:
    /** Constructs a new character table. */
    ExtendedCharTable();
    ~ExtendedCharTable();

    /**
     * Adds a sequences of unicode characters to the table and returns
     * a hash code which can be used later to look up the sequence
     * using lookupExtendedChar()
     *
     * If the same sequence already exists in the table, the hash
     * of the existing sequence will be returned.
     *
     * @param unicodePoints An array of unicode character points
     * @param length Length of @p unicodePoints
     */
    ushort createExtendedChar(ushort* unicodePoints, ushort length);
    /**
     * Looks up and returns a pointer to a sequence of unicode characters
     * which was added to the table using createExtendedChar().
     *
     * @param hash The hash key returned by createExtendedChar()
     * @param length This variable is set to the length of the
     * character sequence.
     *
     * @return A unicode character sequence of size @p length.
     */
    ushort* lookupExtendedChar(ushort hash, ushort& length) const;

    /** The global ExtendedCharTable instance. */
    static ExtendedCharTable instance;
private:
    // calculates the hash key of a sequence of unicode points of size 'length'
    ushort extendedCharHash(ushort* unicodePoints, ushort length) const;
    // tests whether the entry in the table specified by 'hash' matches the
    // character sequence 'unicodePoints' of size 'length'
    bool extendedCharMatch(ushort hash, ushort* unicodePoints, ushort length) const;
    // internal, maps hash keys to character sequence buffers.  The first ushort
    // in each value is the length of the buffer, followed by the ushorts in the buffer
    // themselves.
    QHash<ushort, ushort*> extendedCharTable;
};

#endif // CHARACTER_H

