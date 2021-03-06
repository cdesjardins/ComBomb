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
#ifndef CBLISTWIDGET_H
#define CBLISTWIDGET_H

#include <QListWidget>

class CBListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit CBListWidget(QWidget* parent = 0);
    virtual ~CBListWidget()
    {
    }

private slots:
    void itemChangedSlot(QListWidgetItem* changedItem);
private:
    void addBlankItem();
};

#endif // CBLISTWIDGET_H
