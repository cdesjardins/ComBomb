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
#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"

#define CB_CONFIG_SETTINGS_ROOT         "ConfigDialog/"
#define CB_CONFIG_SETTINGS_PORT         "Port"
#define CB_CONFIG_SETTINGS_COM          CB_CONFIG_SETTINGS_ROOT "ComPorts"
#define CB_CONFIG_SETTINGS_TERM         CB_CONFIG_SETTINGS_ROOT "Terminal"
#define CB_CONFIG_SETTINGS_TABBEDVIEW   CB_CONFIG_SETTINGS_ROOT "TabbedView"
#define CB_CONFIG_SETTINGS_BLACKBACK    CB_CONFIG_SETTINGS_ROOT "BlackBack"
#ifdef WIN32
#define BASE_PORTNAME_1 "COM", 1, 257
#else
#define BASE_PORTNAME_1 "/dev/ttyS", 0, 256
#define BASE_PORTNAME_2 "/dev/ttyUSB", 0, 256
#endif

ConfigDialog::ConfigDialog(QWidget* parent) :
    CBDialog(parent),
    ui(new Ui::ConfigDialog)
{
    QTerminalConfig terminalConfig;
    ui->setupUi(this);

    if (ConfigDialog::getTerminalConfig(&terminalConfig) == true)
    {
        ui->fontComboBox->setCurrentFont(terminalConfig._font);
    }
    on_fontComboBox_currentIndexChanged(0);
    ui->wordSelectionDelimitersLineEdit->setText(terminalConfig._wordSelectionDelimiters);

    populateComPortListWidget();
    ui->mdiRdioButton->setChecked(!getTabbedViewSettings());
    ui->tabsRadioButton->setChecked(getTabbedViewSettings());
    ui->blackRadioButton->setChecked(getBlackBackSettings());
    ui->whiteRadioButton->setChecked(!getBlackBackSettings());
}

void ConfigDialog::setPortListSettings()
{
    QSettings settings;
    QListWidgetItem* item;

    settings.beginWriteArray(CB_CONFIG_SETTINGS_ROOT);
    for (int row = 0; row < ui->comPortsList->count(); row++)
    {
        settings.setArrayIndex(row);
        item = ui->comPortsList->item(row);
        settings.setValue(CB_CONFIG_SETTINGS_PORT, item->text());
    }
    settings.endArray();
}

QStringList ConfigDialog::getPortListSettings()
{
    QStringList comPorts;
    int row = 0;
    QSettings settings;
    QString port;
    settings.beginReadArray(CB_CONFIG_SETTINGS_ROOT);
    do
    {
        settings.setArrayIndex(row++);
        port = settings.value(CB_CONFIG_SETTINGS_PORT).toString();
        if (port.length() > 0)
        {
            comPorts.append(port);
        }
    } while (port.length() > 0);
    settings.endArray();
    if (comPorts.length() == 0)
    {
        comPorts = ConfigDialog::getPortListDefaults(BASE_PORTNAME_1);
#ifdef BASE_PORTNAME_2
        comPorts += ConfigDialog::getPortListDefaults(BASE_PORTNAME_2);
#endif
    }
    return comPorts;
}

bool ConfigDialog::getTabbedViewSettings()
{
    QSettings settings;
    return settings.value(CB_CONFIG_SETTINGS_TABBEDVIEW, false).toBool();
}

bool ConfigDialog::getBlackBackSettings()
{
    QSettings settings;
    return settings.value(CB_CONFIG_SETTINGS_BLACKBACK, true).toBool();
}

QStringList ConfigDialog::getPortListDefaults(QString basePortName, int start, int stop)
{
    QStringList comPorts;

    for (int i = start; i < stop; i++)
    {
        comPorts.append(basePortName + QString::number(i));
    }
    return comPorts;
}

void ConfigDialog::populateComPortListWidget()
{
    QListWidgetItem* item;
    int row = 0;
    QStringList comPorts = ConfigDialog::getPortListSettings();
    for (QStringList::iterator it = comPorts.begin(); it != comPorts.end(); ++it)
    {
        item = new QListWidgetItem(*it);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->comPortsList->insertItem(row++, item);
    }
}

ConfigDialog::~ConfigDialog()
{
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }
    delete ui;
}

QString ConfigDialog::getSettingsRoot()
{
    return CB_CONFIG_SETTINGS_ROOT;
}

void ConfigDialog::on_buttonBox_accepted()
{
    QByteArray qbytes;
    QSettings settings;
    QTerminalConfig terminalConfig;
    QDataStream q(&qbytes, QIODevice::WriteOnly);
    terminalConfig._wordSelectionDelimiters = ui->wordSelectionDelimitersLineEdit->text().toLocal8Bit().constData();
    terminalConfig._font = ui->fontComboBox->currentFont();
    terminalConfig._font.setPointSize(ui->fontSizeComboBox->currentData().toInt());
    q << terminalConfig;

    settings.setValue(CB_CONFIG_SETTINGS_TERM, qbytes);
    settings.setValue(CB_CONFIG_SETTINGS_TABBEDVIEW, ui->tabsRadioButton->isChecked());
    settings.setValue(CB_CONFIG_SETTINGS_BLACKBACK, ui->blackRadioButton->isChecked());
    MainWindow::getMainWindow()->setInterfaceType();
    setPortListSettings();

    QList<ChildForm*> list = MainWindow::getMainWindow()->findChildren<ChildForm*>();

    foreach(ChildForm * w, list)
    {
        w->applyTerminalConfig(terminalConfig);
    }
}

bool ConfigDialog::getTerminalConfig(QTerminalConfig* terminalConfig)
{
    bool ret = false;
    QSettings settings;
    QByteArray qbytes(settings.value(CB_CONFIG_SETTINGS_TERM).toByteArray());
    if (qbytes.length() > 0)
    {
        QDataStream q(qbytes);
        q >> *terminalConfig;
        ret = true;
    }
    return ret;
}

void ConfigDialog::on_fontComboBox_currentIndexChanged(int )
{
    QList<int> sizes;
    ui->fontComboBox->getAcceptableFontSizes(&sizes);

    ui->fontSizeComboBox->clear();
    for (QList<int>::iterator it = sizes.begin(); it != sizes.end(); it++)
    {
        std::stringstream fontSizeStr;
        fontSizeStr << *it;
        ui->fontSizeComboBox->addItem(fontSizeStr.str().c_str(), *it);
    }
}
