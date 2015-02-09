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
#include "cbfontcombobox.h"
#include "QTerminal/QTerminalInterface.h"

Q_DECLARE_METATYPE(QList<int>)

CBFontComboBox::CBFontComboBox(QWidget* parent)
    : QComboBox(parent)
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
                addItem(family, QVariant::fromValue< QList<int> >(sizes));
                setItemData(count() - 1, QFont(family), Qt::FontRole);
            }
        }
    }
}

CBFontComboBox::~CBFontComboBox()
{

}

QFont CBFontComboBox::currentFont() const
{
    QFont ret(currentText());
    return ret;
}

void CBFontComboBox::setCurrentFont(const QFont& font)
{
    int index = findText(font.family());
    if (index >= 0)
    {
        setCurrentIndex(index);
    }
}

void CBFontComboBox::getAcceptableFontSizes(QList<int>* sizes)
{
    *sizes = itemData(currentIndex()).value< QList<int> >();
}
