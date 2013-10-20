#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"

#define CB_CONFIG_SETTINGS_ROOT "Config/"


ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    QTerminalConfig terminalConfig;
    ui->setupUi(this);

    ConfigDialog::getTerminalConfig(&terminalConfig);
    ui->wordSelectionDelimitersLineEdit->setText(terminalConfig._wordSelectionDelimiters.c_str());
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
    std::stringstream ofs;
    boost::archive::text_oarchive oa(ofs);
    QSettings settings;
    QTerminalConfig terminalConfig;
    terminalConfig._wordSelectionDelimiters = ui->wordSelectionDelimitersLineEdit->text().toLocal8Bit().constData();
    oa << terminalConfig;
    settings.setValue(CB_CONFIG_SETTINGS_ROOT "Settings", ofs.str().c_str());

    QList<ChildForm*> list = MainWindow::getMainWindow()->findChildren<ChildForm *>();
    foreach(ChildForm *w, list)
    {
        w->applyTerminalConfig(terminalConfig);
    }
}

void ConfigDialog::getTerminalConfig(QTerminalConfig *terminalConfig)
{
    QSettings settings;
    std::stringstream s;
    s << settings.value(CB_CONFIG_SETTINGS_ROOT "Settings").toString().toLocal8Bit().constData();
    if (s.str().length() > 0)
    {
        boost::archive::text_iarchive ia(s);
        ia >> *terminalConfig;
    }
}


