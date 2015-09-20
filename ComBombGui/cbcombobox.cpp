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
#include "cbcombobox.h"
#include <QSettings>
#include <QDialog>

CBComboBox::CBComboBox(QWidget* parent) :
    QComboBox(parent),
    _saved(false),
    _restored(false)
{
}

CBComboBox::~CBComboBox()
{
}

QString CBComboBox::getName()
{
    QString ret;
    QWidget* w = parentWidget();
    for (int i = 0; ((i < 15) && (w != nullptr)); i++)
    {
        QDialog* d = qobject_cast<QDialog*>(w);
        if (d != nullptr)
        {
            ret = (d->objectName() + "/" + objectName());
            break;
        }
        w = w->parentWidget();
    }
    return ret;
}

void CBComboBox::showEvent(QShowEvent*)
{
    restoreComboBox();
}

void CBComboBox::hideEvent(QHideEvent*)
{
    saveComboBox();
}

void CBComboBox::saveComboBox()
{
    if (_saved == false)
    {
        _saved = true;
        if (isEditable() == true)
        {
            saveEditableComboBox();
        }
        else
        {
            saveStaticComboBox();
        }
    }
}

void CBComboBox::saveStaticComboBox()
{
    QSettings settings;
    QString name = getName();
    settings.setValue(name, currentText());
}

void CBComboBox::saveEditableComboBox()
{
    QSettings settings;
    QStringList itemList;
    QString name = getName();
    int i;
    itemList.append(currentText());
    for (i = 0; i < count(); i++)
    {
        itemList.append(itemText(i));
    }
    itemList.removeDuplicates();
    settings.beginWriteArray(name);
    i = 0;
    for (QStringList::iterator it = itemList.begin(); it != itemList.end(); it++)
    {
        if ((it->length() > 0) || (it == itemList.begin()))
        {
            settings.setArrayIndex(i++);
            settings.setValue("Item", *it);
        }
    }
    settings.endArray();
}

void CBComboBox::restoreComboBox()
{
    if (_restored == false)
    {
        _restored = true;
        if (isEditable() == true)
        {
            restoreEditableComboBox();
        }
        else
        {
            restoreStaticComboBox();
        }
    }
}

void CBComboBox::restoreStaticComboBox()
{
    QSettings settings;
    QString name = getName();
    QString value = settings.value(name).toString();
    if (value.length() == 0)
    {
        value = _default;
    }
    if (value.length() > 0)
    {
        int index = findText(value);
        if (index >= 0)
        {
            setCurrentIndex(index);
        }
    }
}

void CBComboBox::restoreEditableComboBox()
{
    QSettings settings;
    QString name = getName();
    int size = settings.beginReadArray(name);
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        addItem(settings.value("Item").toString());
    }
    settings.endArray();
}

void CBComboBox::addOrUpdateItem(const QString& item)
{
    int index = findText(item);
    QString tmpItem;

    if (index == -1)
    {
        tmpItem = item;
    }
    else
    {
        tmpItem = itemText(index);
        removeItem(index);
    }
    insertItem(0, tmpItem);
    setCurrentIndex(0);
}

void CBComboBox::setDefault(const QString& defaultVal)
{
    _default = defaultVal;
}

