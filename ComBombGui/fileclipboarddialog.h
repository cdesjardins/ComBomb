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
#ifndef FILECLIPBOARDDIALOG_H
#define FILECLIPBOARDDIALOG_H

#include "cbdialog.h"
#include "fileclipboardheader.h"

namespace Ui {
class FileClipboardDialog;
}

class FileClipboardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileClipboardDialog(QWidget* parent = 0);
    virtual ~FileClipboardDialog();

protected:
    void loadFileClipboardSettings();
    void saveFileClipboardSetting(int row);
    void loadNewLineSettings();
    void keyPressEvent(QKeyEvent* e);

private slots:
    void sendItemTriggered(int index);
    void on_fileClipboardTable_cellChanged(int row, int column);
    void on_newLineCheckBox_toggled(bool checked);
    void on_sendButton_clicked();
    void on_searchButton_clicked();

private:
    Ui::FileClipboardDialog* ui;
    FileClipboardHeader* _fileClipboardHeader;
    bool _fileClipboardLoaded;
};

#endif // FILECLIPBOARDDIALOG_H
