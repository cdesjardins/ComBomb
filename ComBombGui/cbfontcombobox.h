/*
    ComBomb - Terminal emulator
    Copyright (C) 2015  Chris Desjardins
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
#ifndef CBFONTCOMBOBOX_H
#define CBFONTCOMBOBOX_H

#include <QFontComboBox>

class CBFontComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CBFontComboBox(QWidget* parent = 0);
    virtual ~CBFontComboBox();
    QFont currentFont() const;
    void setCurrentFont(const QFont& font);
    void getAcceptableFontSizes(QList<int>* sizes);
private:
};

#endif // CBFONTCOMBOBOX_H
