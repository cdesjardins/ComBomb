/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    http://blog.chrisd.info cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "QTerminal/QTerminalConfig.h"

QTerminalConfig::QTerminalConfig()
    : _wordSelectionDelimiters("@-./_~")
{
    _font.setStyleHint(QFont::Courier);
}

QDataStream &operator<<(QDataStream &out, const QTerminalConfig &q)
{
    out << q._wordSelectionDelimiters;
    out << q._font;
    return out;
}

QDataStream &operator>>(QDataStream &in, QTerminalConfig &q)
{
    q = QTerminalConfig();
    in >> q._wordSelectionDelimiters;
    in >> q._font;
    return in;
}
