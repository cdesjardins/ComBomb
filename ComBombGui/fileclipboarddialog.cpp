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
#include <QSettings>
#include "fileclipboarddialog.h"
#include "ui_fileclipboarddialog.h"
#include "mainwindow.h"

#define CB_FILE_CLIBBOARD_SETTINGS_ROOT "FileClipboard/"

FileClipboardDialog::FileClipboardDialog(QWidget* parent) :
    CBDialog(parent),
    ui(new Ui::FileClipboardDialog),
    _fileClipboardHeader(NULL),
    _fileClipboardLoaded(false)
{
    ui->setupUi(this);

    loadFileClipboardSettings();
    loadNewLineSettings();

    _fileClipboardHeader = new FileClipboardHeader();
    ui->fileClipboardTable->setVerticalHeader(_fileClipboardHeader);

    connect(_fileClipboardHeader, SIGNAL(sendItemSignal(int)), this, SLOT(sendItemTriggered(int)));
    ui->fileClipboardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _fileClipboardLoaded = true;
}

FileClipboardDialog::~FileClipboardDialog()
{
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }

    delete ui;
    if (_fileClipboardHeader != NULL)
    {
        delete _fileClipboardHeader;
    }
}

QString FileClipboardDialog::getSettingsRoot()
{
    return CB_FILE_CLIBBOARD_SETTINGS_ROOT;
}

void FileClipboardDialog::saveFileClipboardSetting(int row)
{
    QTableWidgetItem* item;
    QSettings settings;
    settings.beginWriteArray(CB_FILE_CLIBBOARD_SETTINGS_ROOT);
    settings.setArrayIndex(row);
    item = ui->fileClipboardTable->item(row, 0);
    settings.setValue("Text", item->text());
    settings.endArray();
}

void FileClipboardDialog::loadFileClipboardSettings()
{
    QSettings settings;
    QTableWidgetItem* item;
    settings.beginReadArray(CB_FILE_CLIBBOARD_SETTINGS_ROOT);
    for (int row = 0; row < 256; row++)
    {
        settings.setArrayIndex(row);
        item = new QTableWidgetItem(settings.value("Text").toString());
        ui->fileClipboardTable->setItem(row, 0, item);
    }
    settings.endArray();
}

void FileClipboardDialog::sendItemTriggered(int index)
{
    QTableWidgetItem* item = ui->fileClipboardTable->item(index, 0);
    if ((item != NULL) && (item->text().length() > 0))
    {
        ChildForm* c = MainWindow::getMainWindow()->getActiveChildWindow();
        if (c != NULL)
        {
            QString text = item->text();
            if (ui->newLineCheckBox->isChecked())
            {
                text += "\n";
            }

            c->sendText(text);
            c->setTrackOutput(true);
            QApplication::setActiveWindow(MainWindow::getMainWindow());
        }
    }
}

void FileClipboardDialog::on_fileClipboardTable_cellChanged(int row, int column)
{
    UNREF_PARAM(column);
    if (_fileClipboardLoaded == true)
    {
        saveFileClipboardSetting(row);
    }
}

void FileClipboardDialog::on_newLineCheckBox_toggled(bool checked)
{
    QSettings settings;
    settings.setValue(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine", checked);
}

void FileClipboardDialog::loadNewLineSettings()
{
    QSettings settings;
    bool sendNewLineChecked = settings.value(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine").toBool();
    ui->newLineCheckBox->setChecked(sendNewLineChecked);
}

void FileClipboardDialog::on_sendButton_clicked()
{
    sendItemTriggered(ui->fileClipboardTable->currentRow());
}

