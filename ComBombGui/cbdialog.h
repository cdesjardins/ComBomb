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
#ifndef CBDIALOG_H
#define CBDIALOG_H

#include <QDialog>

class CBDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CBDialog(QWidget *parent = 0);
    virtual ~CBDialog() {}
protected:
    virtual QString getSettingsRoot() = 0;
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

signals:

public slots:

};

#endif // CBDIALOG_H
