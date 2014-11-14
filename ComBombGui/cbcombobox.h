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
#ifndef CBCOMBOBOX_H
#define CBCOMBOBOX_H

#include <QComboBox>

class CBComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CBComboBox(QWidget* parent = 0);
    virtual ~CBComboBox();
    void addOrUpdateItem(const QString &item);
    void setDefault(const QString &defaultVal);
    void restoreComboBox();
    void saveComboBox();
signals:

public slots:

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);
    QString getName();
    void saveEditableComboBox();
    void saveStaticComboBox();

    void restoreEditableComboBox();
    void restoreStaticComboBox();
private:
    QString _default;
    bool _saved;
    bool _restored;
};

#endif // CBCOMBOBOX_H
