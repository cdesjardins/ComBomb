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
#include "cblistwidget.h"

CBListWidget::CBListWidget(QWidget * parent)
    : QListWidget(parent)
{
    addBlankItem();
    connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChangedSlot(QListWidgetItem*)));
}

void CBListWidget::itemChangedSlot(QListWidgetItem* changedItem)
{
    QListWidgetItem* lastItem = item (count() - 1);
    if (lastItem == changedItem)
    {
        if (changedItem->text().length() > 0)
        {
            addBlankItem();
        }
    }
    else
    {
        if (changedItem->text().length() == 0)
        {
            delete takeItem(row(changedItem));
        }
    }
}

void CBListWidget::addBlankItem()
{
    QListWidgetItem* newBlankItem;
    newBlankItem = new QListWidgetItem("");
    newBlankItem->setFlags(newBlankItem->flags () | Qt::ItemIsEditable);
    addItem(newBlankItem);
}
