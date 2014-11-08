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
#ifndef FILECLIPBOARDHEADER_H
#define FILECLIPBOARDHEADER_H
#include <QHeaderView>

class FileClipboardHeader : public QHeaderView
{
    Q_OBJECT
public:
    FileClipboardHeader()
        : QHeaderView(Qt::Vertical)
    {
        setSectionsClickable(true);
        connect(this, SIGNAL(sectionClicked(int)), this, SLOT(sectionClicked(int)));
    }

    virtual ~FileClipboardHeader()
    {
    }

signals:
    void sendItemSignal(int index);

public slots:
    void sectionClicked(int index)
    {
        emit sendItemSignal(index);
    }
};
#endif // FILECLIPBOARDHEADER_H
