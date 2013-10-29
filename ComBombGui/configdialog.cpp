#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"

#define CB_CONFIG_SETTINGS_ROOT "ConfigDialog/"


ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    QTerminalConfig terminalConfig;
    ui->setupUi(this);

    ConfigDialog::getTerminalConfig(&terminalConfig);
    ui->wordSelectionDelimitersLineEdit->setText(terminalConfig._wordSelectionDelimiters);
}

ConfigDialog::~ConfigDialog()
{
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }
    delete ui;
}

void ConfigDialog::hideEvent(QHideEvent*)
{
    MainWindow::saveWidgetGeometry(this, CB_CONFIG_SETTINGS_ROOT "Geometry");
}

void ConfigDialog::showEvent(QShowEvent*)
{
    MainWindow::restoreWidgetGeometry(this, CB_CONFIG_SETTINGS_ROOT "Geometry");
}

void ConfigDialog::on_buttonBox_accepted()
{
    QByteArray qbytes;
    QSettings settings;
    QTerminalConfig terminalConfig;
    QDataStream q(&qbytes, QIODevice::WriteOnly);
    terminalConfig._wordSelectionDelimiters = ui->wordSelectionDelimitersLineEdit->text().toLocal8Bit().constData();
    q << terminalConfig;

    settings.setValue(CB_CONFIG_SETTINGS_ROOT "Settings", qbytes);

    QList<ChildForm*> list = MainWindow::getMainWindow()->findChildren<ChildForm *>();
    foreach(ChildForm *w, list)
    {
        w->applyTerminalConfig(terminalConfig);
    }
}

void ConfigDialog::getTerminalConfig(QTerminalConfig *terminalConfig)
{
    QSettings settings;
    QByteArray qbytes(settings.value(CB_CONFIG_SETTINGS_ROOT "Settings").toByteArray());
    if (qbytes.length() > 0)
    {
        QDataStream q(qbytes);
        q >> *terminalConfig;
    }
}


