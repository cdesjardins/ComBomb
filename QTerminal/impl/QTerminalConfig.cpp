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
#include "../QTerminalConfig.h"
#include "QTerminalInterface.h"
#include <QFontDatabase>
#include <QStringList>

#define CP_TERM_CFG_VER_1       1
#define CP_TERM_CFG_VER_2       2 // added history size
#define CP_TERM_CFG_LATEST      CP_TERM_CFG_VER_2

QTerminalConfig::QTerminalConfig()
    : _wordSelectionDelimiters("@-./_~"),
      _histSize(500000)
{
    QFontDatabase database;

    foreach(const QString &family, database.families())
    {
        QFont f(family);
        QList<int> sizes;
        if (QTerminalInterface::findAcceptableFontSizes(f, &sizes) == true)
        {
            _font = QFont(f.family(), sizes[sizes.size() / 2]);
            break;
        }
    }
}

QDataStream& operator<<(QDataStream& out, const QTerminalConfig& q)
{
    out << CP_TERM_CFG_LATEST;
    out << q._wordSelectionDelimiters;
    out << q._font.family();
    out << q._font.pointSize();
    out << q._histSize;
    return out;
}

void QTerminalConfig::readCfgV1(QDataStream& in, QTerminalConfig& q)
{
    QString family;
    int pointSize;
    QFont testFont;
    QList<int> sizes;

    in >> q._wordSelectionDelimiters;
    in >> family;
    in >> pointSize;

    testFont.setFamily(family);
    testFont.setPointSize(pointSize);
    if (QTerminalInterface::findAcceptableFontSizes(testFont, &sizes) == true)
    {
        q._font.setFamily(family);
        if (sizes.indexOf(pointSize) != -1)
        {
            q._font.setPointSize(pointSize);
        }
    }
}

void QTerminalConfig::readCfgV2(QDataStream& in, QTerminalConfig& q)
{
    readCfgV1(in, q);
    in >> q._histSize;
}

QDataStream& operator>>(QDataStream& in, QTerminalConfig& q)
{
    int version;
    q = QTerminalConfig();
    in >> version;
    switch (version)
    {
        case CP_TERM_CFG_VER_1:
            QTerminalConfig::readCfgV1(in, q);
            break;
        case CP_TERM_CFG_VER_2:
            QTerminalConfig::readCfgV2(in, q);
            break;
    }
    return in;
}

