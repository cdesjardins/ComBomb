#include <QFileDialog>
#include <QSettings>
#include "mainwindow.h"
#include "runprocessdialog.h"
#include "ui_runprocessdialog.h"

#define CB_RUN_PROCESS_SETTINGS_ROOT    objectName()
#define CB_RUN_PROCESS_SETTINGS_PB      CB_RUN_PROCESS_SETTINGS_ROOT + "/Process/Browser"
#define CB_RUN_PROCESS_SETTINGS_WDB     CB_RUN_PROCESS_SETTINGS_ROOT + "/WorkingDir/Browser"
#define CB_RUN_PROCESS_SETTINGS_OUTPUT     CB_RUN_PROCESS_SETTINGS_ROOT + "/SupressOutput"

RunProcessDialog::RunProcessDialog(QWidget *parent) :
    CBDialog(parent),
    ui(new Ui::RunProcessDialog)
{
    QSettings settings;
    ui->setupUi(this);
    ui->suppressOutputCheckBox->setChecked(settings.value(CB_RUN_PROCESS_SETTINGS_OUTPUT, true).toBool());
}

RunProcessDialog::~RunProcessDialog()
{
    QSettings settings;
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

bool RunProcessDialog::isOutputSuppressed()
{
    return ui->suppressOutputCheckBox->isChecked();
}
