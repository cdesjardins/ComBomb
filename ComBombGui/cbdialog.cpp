/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    https://github.com/cdesjardins/ComBomb cjd@chrisd.info

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
#include "cbdialog.h"
#include "cbcombobox.h"
#include <QSettings>

CBDialog::CBDialog(QWidget* parent) :
    QDialog(parent)
{
}

void CBDialog::showEvent(QShowEvent*)
{
    QSettings settings;
    restoreGeometry(settings.value(getSettingsRoot() + "Geometry").toByteArray());
    // Drive CBComboBox restore at dialog level so combos in non-active tab
    // pages also load their persisted history. Per-widget showEvent only
    // fires on currently visible widgets, missing the inactive tabs.
    for (CBComboBox* cb : findChildren<CBComboBox*>())
    {
        cb->restoreComboBox();
    }
}

void CBDialog::hideEvent(QHideEvent*)
{
    QSettings settings;
    settings.setValue(getSettingsRoot() + "Geometry", saveGeometry());
    for (CBComboBox* cb : findChildren<CBComboBox*>())
    {
        cb->saveComboBox();
    }
}
