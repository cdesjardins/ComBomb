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
#include "finddialog.h"
#include "ui_finddialog.h"
#include <QSettings>

#define CB_FIND_CASE_SENSITIVE          getSettingsRoot() + "/CaseSensitive"
#define CB_FIND_SEARCH_UP               getSettingsRoot() + "/SearchUp"

FindDialog::FindDialog(QWidget* parent) :
    CBDialog(parent),
    ui(new Ui::FindDialog)
{
    QSettings settings;
    ui->setupUi(this);
    ui->caseSensitiveCheckBox->setChecked(settings.value(CB_FIND_CASE_SENSITIVE, false).toBool());
    ui->searchUpCheckBox->setChecked(settings.value(CB_FIND_SEARCH_UP, false).toBool());
    ui->findWhatComboBox->restoreComboBox();
}

FindDialog::~FindDialog()
{
    QSettings settings;
    settings.setValue(CB_FIND_CASE_SENSITIVE, ui->caseSensitiveCheckBox->isChecked());
    settings.setValue(CB_FIND_SEARCH_UP, ui->searchUpCheckBox->isChecked());
    ui->findWhatComboBox->saveComboBox();
    delete ui;
}

QString FindDialog::getString()
{
    return ui->findWhatComboBox->currentText();
}

void FindDialog::addString(const QString& newStr)
{
    ui->findWhatComboBox->setCurrentText(newStr);
}

bool FindDialog::getCaseSensitivity()
{
    return ui->caseSensitiveCheckBox->isChecked();
}

bool FindDialog::getSearchUp()
{
    return ui->searchUpCheckBox->isChecked();
}

void FindDialog::setSearchUp(bool searchUp)
{
    ui->searchUpCheckBox->setChecked(searchUp);
}

QString FindDialog::getSettingsRoot()
{
    return objectName();
}

