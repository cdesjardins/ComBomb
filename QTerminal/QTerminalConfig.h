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
#ifndef QTERMINALCONFIG_H
#define QTERMINALCONFIG_H

#include <QString>
#include <QDataStream>
#include <QFont>
#include "unparam.h"

class QTerminalConfig
{
public:
    QTerminalConfig();
    QString _wordSelectionDelimiters;
    QFont _font;
};

QDataStream &operator<<(QDataStream &out, const QTerminalConfig &q);
QDataStream &operator>>(QDataStream &in, QTerminalConfig &q);

#endif // QTERMINALCONFIG_H
