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
#include <QFileDialog>
#include <QSettings>
#include "capturedialog.h"
#include "ui_capturedialog.h"

#define CB_CAPTURE_SETTINGS_BROWSE_DIR      getSettingsRoot() + "/Filename/Browser"

CaptureDialog::CaptureDialog(QWidget *parent) :
    CBDialog(parent),
    ui(new Ui::CaptureDialog)
{
    ui->setupUi(this);
}

CaptureDialog::~CaptureDialog()
{
    delete ui;
}

QString CaptureDialog::getSettingsRoot()
{
    return objectName();
}

QString CaptureDialog::getCaptureFilename()
{
    return ui->captureFileComboBox->currentText();
}

void CaptureDialog::on_pushButton_clicked()
{
    QSettings settings;
    QString fileName;
    QString dirName = settings.value(CB_CAPTURE_SETTINGS_BROWSE_DIR, QString()).toString();
    fileName = QFileDialog::getOpenFileName(this, tr("Programs"), dirName, tr("All files (*.*)"));
    if (fileName.isNull() == false)
    {
        settings.setValue(CB_CAPTURE_SETTINGS_BROWSE_DIR, QFileInfo(fileName).canonicalPath());
        ui->captureFileComboBox->addOrUpdateItem(fileName);
    }
}
