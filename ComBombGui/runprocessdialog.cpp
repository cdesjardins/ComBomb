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
#include <QFileDialog>
#include <QSettings>
#include "mainwindow.h"
#include "runprocessdialog.h"
#include "ui_runprocessdialog.h"

#define CB_RUN_PROCESS_SETTINGS_ROOT    objectName()
#define CB_RUN_PROCESS_SETTINGS_PB      CB_RUN_PROCESS_SETTINGS_ROOT + "/Process/Browser"
#define CB_RUN_PROCESS_SETTINGS_WDB     CB_RUN_PROCESS_SETTINGS_ROOT + "/WorkingDir/Browser"
#define CB_RUN_PROCESS_SETTINGS_STDOUT  CB_RUN_PROCESS_SETTINGS_ROOT + "/StdoutRedir"
#define CB_RUN_PROCESS_SETTINGS_STDERR  CB_RUN_PROCESS_SETTINGS_ROOT + "/StderrRedir"
#define CB_RUN_PROCESS_SETTINGS_OUTPUT  CB_RUN_PROCESS_SETTINGS_ROOT + "/SupressOutput"

RunProcessDialog::RunProcessDialog(QWidget* parent) :
    CBDialog(parent),
    ui(new Ui::RunProcessDialog)
{
    QSettings settings;
    ui->setupUi(this);
    ui->stdoutCheckBox->setChecked(settings.value(CB_RUN_PROCESS_SETTINGS_STDOUT, true).toBool());
    ui->stderrCheckBox->setChecked(settings.value(CB_RUN_PROCESS_SETTINGS_STDERR, false).toBool());
    ui->suppressOutputCheckBox->setChecked(settings.value(CB_RUN_PROCESS_SETTINGS_OUTPUT, false).toBool());
}

RunProcessDialog::~RunProcessDialog()
{
    QSettings settings;
    settings.setValue(CB_RUN_PROCESS_SETTINGS_STDOUT, ui->stdoutCheckBox->isChecked());
    settings.setValue(CB_RUN_PROCESS_SETTINGS_STDERR, ui->stderrCheckBox->isChecked());
    settings.setValue(CB_RUN_PROCESS_SETTINGS_OUTPUT, ui->suppressOutputCheckBox->isChecked());
    delete ui;
}

QString RunProcessDialog::getSettingsRoot()
{
    return CB_RUN_PROCESS_SETTINGS_ROOT;
}

void RunProcessDialog::on_programBrowseButton_clicked()
{
    QSettings settings;
    QString fileName;
    QString dirName = settings.value(CB_RUN_PROCESS_SETTINGS_PB, QString()).toString();
    fileName = QFileDialog::getOpenFileName(this, tr("Programs"), dirName, tr("All files (*)"));
    if (fileName.isNull() == false)
    {
        settings.setValue(CB_RUN_PROCESS_SETTINGS_PB, QFileInfo(fileName).canonicalPath());
        ui->programComboBox->addOrUpdateItem(fileName);
    }
}

void RunProcessDialog::on_workingDirBrowseButton_clicked()
{
    QSettings settings;
    QString dirName = settings.value(CB_RUN_PROCESS_SETTINGS_WDB, QString()).toString();
    dirName = QFileDialog::getExistingDirectory(this, tr("Working directory"), dirName);
    if (dirName.isNull() == false)
    {
        settings.setValue(CB_RUN_PROCESS_SETTINGS_WDB, dirName);
        ui->workingDirComboBox->addOrUpdateItem(dirName);
    }
}

QString RunProcessDialog::getWorkingDirectory()
{
    return ui->workingDirComboBox->currentText();
}

QStringList RunProcessDialog::getArguments()
{
    QString args = ui->argumentsComboBox->currentText();

    return args.split(' ');
}

QString RunProcessDialog::getProgram()
{
    return ui->programComboBox->currentText();
}

bool RunProcessDialog::isStdoutRedirected()
{
    return ui->stdoutCheckBox->isChecked();
}

bool RunProcessDialog::isStderrRedirected()
{
    return ui->stderrCheckBox->isChecked();
}

bool RunProcessDialog::isOutputSuppressed()
{
    return ui->suppressOutputCheckBox->isChecked();
}
