#include "opendialog.h"
#include "ui_opendialog.h"
#include <sstream>
#include <QFileDialog>
#include <QIntValidator>
#ifndef Q_MOC_RUN
#include <boost/asio/serial_port.hpp>
#endif

#ifdef WIN32
#define BASE_PORTNAME_1 "COM"
#else
#define BASE_PORTNAME_1 "/dev/ttyS"
#define BASE_PORTNAME_2 "/dev/ttyUSB"
#endif


OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
    addComPorts(BASE_PORTNAME_1);
#ifdef BASE_PORTNAME_2
    addComPorts(BASE_PORTNAME_2);
#endif
    addBaudRates();
    addParity();
    addStopBits();
    addByteSize();
    addFlowControl();
    ui->_fileNameTextBox->setText("C:/Users/ChrisD/software_devel/ComBomb-build-Desktop_Qt_5_0_1_MSVC2010_32bit-Debug/test.cbd");
    ui->_portNumLineEdit->setValidator(new QIntValidator(0, 65536, this));
}

OpenDialog::ConnectionType OpenDialog::getConnectionType()
{
    ConnectionType ret = (ConnectionType)ui->tabWidget->currentIndex();
    return ret;
}

const TgtFileIntf::TgtConnection OpenDialog::getFileConfig() const
{
    TgtFileIntf::TgtConnection ret(ui->_fileNameTextBox->text().toLocal8Bit().constData());
    return ret;
}

const TgtSshIntf::TgtConnection OpenDialog::getSshConfig() const
{
    TgtSshIntf::TgtConnection ret(ui->_hostNameComboBox->currentText().toLocal8Bit().constData(), ui->_portNumLineEdit->text().toInt());
    return ret;
}

const TgtSerialIntf::TgtConnection OpenDialog::getSerialConfig() const
{
    QVariant vBaudRate    = ui->_baudRateComboBox->itemData(ui->_baudRateComboBox->currentIndex());
    QVariant vParity      = ui->_parityComboBox->itemData(ui->_parityComboBox->currentIndex());
    QVariant vStopBits    = ui->_stopBitsComboBox->itemData(ui->_stopBitsComboBox->currentIndex());
    QVariant vByteSize    = ui->_byteSizeComboBox->itemData(ui->_byteSizeComboBox->currentIndex());
    QVariant vFlowControl = ui->_flowControlComboBox->itemData(ui->_flowControlComboBox->currentIndex());

    TgtSerialIntf::TgtConnection ret(
        ui->_comPortComboBox->currentText().toUtf8().constData(),
        boost::asio::serial_port_base::baud_rate(vBaudRate.toInt()),
        boost::asio::serial_port_base::parity((boost::asio::serial_port_base::parity::type)vParity.toInt()),
        boost::asio::serial_port_base::stop_bits((boost::asio::serial_port_base::stop_bits::type)vStopBits.toInt()),
        boost::asio::serial_port_base::character_size(vByteSize.toInt()),
        boost::asio::serial_port_base::flow_control((boost::asio::serial_port_base::flow_control::type)vFlowControl.toInt()));
    return ret;
}

OpenDialog::~OpenDialog()
{
    delete ui;
}

void OpenDialog::addFlowControl()
{
    ui->_flowControlComboBox->addItem("None", boost::asio::serial_port_base::flow_control::none);
    ui->_flowControlComboBox->addItem("Hardware", boost::asio::serial_port_base::flow_control::hardware);
    ui->_flowControlComboBox->addItem("Software", boost::asio::serial_port_base::flow_control::software);
}

void OpenDialog::addStopBits()
{
    ui->_stopBitsComboBox->addItem("1", boost::asio::serial_port_base::stop_bits::one);
    ui->_stopBitsComboBox->addItem("1.5", boost::asio::serial_port_base::stop_bits::onepointfive);
    ui->_stopBitsComboBox->addItem("2", boost::asio::serial_port_base::stop_bits::two);
}

void OpenDialog::addByteSize()
{
    ui->_byteSizeComboBox->addItem("8", 8);
    ui->_byteSizeComboBox->addItem("7", 7);
}

void OpenDialog::addParity()
{
    ui->_parityComboBox->addItem("None", boost::asio::serial_port_base::parity::none);
    ui->_parityComboBox->addItem("Even", boost::asio::serial_port_base::parity::even);
    ui->_parityComboBox->addItem("Odd", boost::asio::serial_port_base::parity::odd);
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
    for (int i = 0; i < (sizeof(bauds) / sizeof(bauds[0])); i++)
    {
        baudRate.str("");
        baudRate << bauds[i];
        ui->_baudRateComboBox->addItem(baudRate.str().c_str(), bauds[i]);
    }
}

void OpenDialog::addComPorts(const std::string &basePortName)
{
    std::stringstream portName;
    boost::asio::io_service ioService;
    boost::system::error_code ec;
    for (int i = 0; i < 16; i++)
    {
        portName.str("");
        portName << basePortName << i;
        boost::asio::serial_port port(ioService);
        qDebug("Serial port: %s", portName.str().c_str());
        port.open(portName.str(), ec);
        if (port.is_open() == true)
        {
            ui->_comPortComboBox->addItem(portName.str().c_str());
        }
    }
}

void OpenDialog::on__browseButton_clicked()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("All files"), NULL, tr("All files (*.*)"));
    ui->_fileNameTextBox->setText(fileName);

}
