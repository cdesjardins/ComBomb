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
#include "opendialog.h"
#include "ui_opendialog.h"
#include "configdialog.h"
#include <sstream>
#include <QFileDialog>
#include <QIntValidator>
#include <QSettings>
#include <boost/asio/serial_port.hpp>

#define CB_OPEN_SETTINGS_ROOT   "OpenDialog/"
#define CB_OPEN_CONN_TYPE       CB_OPEN_SETTINGS_ROOT "ConnectionType"
#define CB_OPEN_CONN_CRLF       CB_OPEN_SETTINGS_ROOT "NewLines"
#define CB_OPEN_CONN_PB         CB_OPEN_SETTINGS_ROOT "Process/Browser"
#define CB_OPEN_CONN_WDB        CB_OPEN_SETTINGS_ROOT "WorkingDir/Browser"
#define CB_OPEN_CONN_X11        CB_OPEN_SETTINGS_ROOT "X11Forwarding"

OpenDialog::OpenDialog(QWidget* parent) :
    CBDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
    addComPorts();
    addBaudRates();
    addParity();
    addStopBits();
    addByteSize();
    addFlowControl();
    ui->portNumLineEdit->setValidator(new QIntValidator(0, 65536, this));
    QSettings settings;
    ui->tabWidget->setCurrentIndex(settings.value(CB_OPEN_CONN_TYPE, CB_CONN_SSH).toInt());
    ui->newLineCheckBox->setChecked(settings.value(CB_OPEN_CONN_CRLF, false).toBool());
    ui->x11ForwardingCheckBox->setChecked(settings.value(CB_OPEN_CONN_X11, true).toBool());
}

OpenDialog::ConnectionType OpenDialog::getConnectionType()
{
    ConnectionType ret = (ConnectionType)ui->tabWidget->currentIndex();
    return ret;
}

std::shared_ptr<const TgtCppsshIntf::TgtConnectionConfig> OpenDialog::getSshConfig() const
{
    std::shared_ptr<TgtCppsshIntf::TgtConnectionConfig> ret(new TgtCppsshIntf::TgtConnectionConfig(
                                                                ui->hostNameComboBox->currentText().toLocal8Bit().constData(),
                                                                ui->portNumLineEdit->text().toInt(),
                                                                ui->userNameComboBox->currentText().toLocal8Bit().constData(),
                                                                ui->passwordLineEdit->text().toLocal8Bit().constData(),
                                                                ui->privKeyFileComboBox->currentText().toLocal8Bit().constData(),
                                                                ui->x11ForwardingCheckBox->isChecked()));
    return ret;
}

std::shared_ptr<const TgtTelnetIntf::TgtConnectionConfig> OpenDialog::getTelnetConfig() const
{
    std::shared_ptr<TgtTelnetIntf::TgtConnectionConfig> ret(new TgtTelnetIntf::TgtConnectionConfig(
                                                                ui->telnetHostNameComboBox->currentText().toLocal8Bit().constData(),
                                                                ui->telnetPortNumLineEdit->text().toInt()));
    return ret;
}

std::shared_ptr<const TgtSerialIntf::TgtConnectionConfig> OpenDialog::getSerialConfig() const
{
    QVariant vBaudRate    = ui->baudRateComboBox->itemData(ui->baudRateComboBox->currentIndex());
    QVariant vParity      = ui->parityComboBox->itemData(ui->parityComboBox->currentIndex());
    QVariant vStopBits    = ui->stopBitsComboBox->itemData(ui->stopBitsComboBox->currentIndex());
    QVariant vByteSize    = ui->byteSizeComboBox->itemData(ui->byteSizeComboBox->currentIndex());
    QVariant vFlowControl = ui->flowControlComboBox->itemData(ui->flowControlComboBox->currentIndex());

    std::shared_ptr<TgtSerialIntf::TgtConnectionConfig> ret(new TgtSerialIntf::TgtConnectionConfig(
                                                                ui->comPortComboBox->currentText().toUtf8().constData(),
                                                                boost::asio::serial_port_base::baud_rate(
                                                                    vBaudRate.toInt()),
                                                                boost::asio::serial_port_base::parity(
                                                                    (boost::asio::serial_port_base::parity::type)vParity.toInt()),
                                                                boost::asio::serial_port_base::stop_bits(
                                                                    (boost::asio::serial_port_base::stop_bits::type)vStopBits.toInt()),
                                                                boost::asio::serial_port_base::character_size(
                                                                    vByteSize.toInt()),
                                                                boost::asio::serial_port_base::flow_control(
                                                                    (boost::asio::serial_port_base::flow_control::type)vFlowControl.toInt())));
    return ret;
}

std::shared_ptr<const TgtProcessIntf::TgtConnectionConfig> OpenDialog::getProcessConfig() const
{
    std::shared_ptr<TgtProcessIntf::TgtConnectionConfig> ret(new TgtProcessIntf::TgtConnectionConfig(
                                                                 ui->programComboBox->currentText().toLocal8Bit().constData(),
                                                                 ui->workingDirComboBox->currentText().toLocal8Bit().constData(),
                                                                 ui->argumentsComboBox->currentText().toLocal8Bit().constData()));
    return ret;
}

OpenDialog::~OpenDialog()
{
    delete ui;
}

void OpenDialog::addFlowControl()
{
    ui->flowControlComboBox->addItem("None", boost::asio::serial_port_base::flow_control::none);
    ui->flowControlComboBox->addItem("Hardware", boost::asio::serial_port_base::flow_control::hardware);
    ui->flowControlComboBox->addItem("Software", boost::asio::serial_port_base::flow_control::software);
}

void OpenDialog::addStopBits()
{
    ui->stopBitsComboBox->addItem("1", boost::asio::serial_port_base::stop_bits::one);
    ui->stopBitsComboBox->addItem("1.5", boost::asio::serial_port_base::stop_bits::onepointfive);
    ui->stopBitsComboBox->addItem("2", boost::asio::serial_port_base::stop_bits::two);
}

void OpenDialog::addByteSize()
{
    ui->byteSizeComboBox->addItem("8", 8);
    ui->byteSizeComboBox->addItem("7", 7);
}

void OpenDialog::addParity()
{
    ui->parityComboBox->addItem("None", boost::asio::serial_port_base::parity::none);
    ui->parityComboBox->addItem("Even", boost::asio::serial_port_base::parity::even);
    ui->parityComboBox->addItem("Odd", boost::asio::serial_port_base::parity::odd);
}

void OpenDialog::addBaudRates()
{
    int bauds[] =
    {
        115200,
        57600,
        38400,
        19200,
        14400,
        9600,
        4800,
        2400,
        1200,
        600,
        300,
        110
    };
    std::stringstream baudRate;
    for (size_t i = 0; i < (sizeof(bauds) / sizeof(bauds[0])); i++)
    {
        baudRate.str("");
        baudRate << bauds[i];
        ui->baudRateComboBox->addItem(baudRate.str().c_str(), bauds[i]);
    }
}

void OpenDialog::addComPorts()
{
    boost::asio::io_service ioService;
    boost::system::error_code ec;
    QStringList comPorts = ConfigDialog::getPortListSettings();
    for (QStringList::iterator it = comPorts.begin(); it != comPorts.end(); ++it)
    {
        boost::asio::serial_port port(ioService);
        QString portname = *it;
        port.open(portname.toLocal8Bit().constData(), ec);
        if (port.is_open() == true)
        {
            ui->comPortComboBox->addItem(*it);
        }
    }
}

void OpenDialog::on_privKeyBrowseButton_clicked()
{
    QString fileName;
    QString dirName = ui->privKeyFileComboBox->currentText();
    if (dirName.isEmpty() == true)
    {
        dirName = QDir::homePath() + "/.ssh";
    }
    fileName = QFileDialog::getOpenFileName(this, tr("Private key files"), dirName, tr("All files (*)"));
    if (fileName.isNull() == false)
    {
        ui->privKeyFileComboBox->insertItem(0, fileName);
        ui->privKeyFileComboBox->setCurrentIndex(0);
    }
}

void OpenDialog::on__buttonBox_accepted()
{
    QSettings settings;
    settings.setValue(CB_OPEN_CONN_TYPE, getConnectionType());
    settings.setValue(CB_OPEN_CONN_CRLF, newlines());
    settings.setValue(CB_OPEN_CONN_X11, ui->x11ForwardingCheckBox->isChecked());
}

bool OpenDialog::newlines()
{
    return ui->newLineCheckBox->isChecked();
}

void OpenDialog::on_programBrowseButton_clicked()
{
    QSettings settings;
    QString fileName;
    QString dirName = settings.value(CB_OPEN_CONN_PB, QString()).toString();
    fileName = QFileDialog::getOpenFileName(this, tr("Programs"), dirName, tr("All files (*)"));
    if (fileName.isNull() == false)
    {
        settings.setValue(CB_OPEN_CONN_PB, QFileInfo(fileName).canonicalPath());
        ui->programComboBox->addOrUpdateItem(fileName);
    }
}

void OpenDialog::on_workingDirButton_clicked()
{
    QSettings settings;
    QString dirName = settings.value(CB_OPEN_CONN_WDB, QString()).toString();
    dirName = QFileDialog::getExistingDirectory(this, tr("Working directory"), dirName);
    if (dirName.isNull() == false)
    {
        settings.setValue(CB_OPEN_CONN_WDB, dirName);
        ui->workingDirComboBox->addOrUpdateItem(dirName);
    }
}

QString OpenDialog::getSettingsRoot()
{
    return objectName();
}

