#include <QFileDialog>
#include <QSettings>
#include "mainwindow.h"
#include "runprocessdialog.h"
#include "ui_runprocessdialog.h"

#define CB_RUN_PROCESS_SETTINGS_ROOT    objectName()
#define CB_RUN_PROCESS_SETTINGS_PB      CB_RUN_PROCESS_SETTINGS_ROOT + "/Process/Browser"
#define CB_RUN_PROCESS_SETTINGS_WDB     CB_RUN_PROCESS_SETTINGS_ROOT + "/WorkingDir/Browser"
#define CB_RUN_PROCESS_SETTINGS_PP      CB_RUN_PROCESS_SETTINGS_ROOT + "/Process/Programs"
#define CB_RUN_PROCESS_SETTINGS_WDP     CB_RUN_PROCESS_SETTINGS_ROOT + "/WorkingDir/Programs"

RunProcessDialog::RunProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunProcessDialog)
{
    ui->setupUi(this);
}

RunProcessDialog::~RunProcessDialog()
{
    delete ui;
}

void RunProcessDialog::on_programBrowseButton_clicked()
{
    QSettings settings;
    QString fileName;
    QString dirName = settings.value(CB_RUN_PROCESS_SETTINGS_PB, QString()).toString();
    fileName = QFileDialog::getOpenFileName(this, tr("Programs"), dirName, tr("Executable files (*.exe);;All files (*.*)"));
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
