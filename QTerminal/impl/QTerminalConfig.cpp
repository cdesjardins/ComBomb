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
#include "QTerminalInterface.h"
#include <QFontDatabase>
#include <QStringList>

#define CP_TERM_CFG_VER_1       1
#define CP_TERM_CFG_LATEST      CP_TERM_CFG_VER_1

QTerminalConfig::QTerminalConfig()
    : _wordSelectionDelimiters("@-./_~")
{
    QFontDatabase database;

    foreach (const QString &family, database.families())
    {
        if (database.isFixedPitch(family) == true)
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
}

QDataStream& operator<<(QDataStream& out, const QTerminalConfig& q)
{
    out << CP_TERM_CFG_LATEST;
    out << q._wordSelectionDelimiters;
    out << q._font.family();
    out << q._font.pointSize();
    return out;
}

QDataStream& operator>>(QDataStream& in, QTerminalConfig& q)
{
    int version;
    q = QTerminalConfig();
    QString family;
    int pointSize;
    in >> version;
    switch (version)
    {
        case CP_TERM_CFG_VER_1:
            in >> q._wordSelectionDelimiters;
            in >> family;
            q._font.setFamily(family);
            in >> pointSize;
            q._font.setPointSize(pointSize);
            break;
    }
    return in;
}

