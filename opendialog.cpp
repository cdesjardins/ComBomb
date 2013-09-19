#include "opendialog.h"
#include "ui_opendialog.h"
#include <sstream>
#ifndef Q_MOC_RUN
#include <boost/asio/serial_port.hpp>
#endif

#ifdef WIN32
#define BASE_PORTNAME "COM"
#else
#define BASE_PORTNAME "/dev/ttyS"
#endif


OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
    detectComPorts();
    for (size_t i = 0; i < _comPorts.size(); i++)
    {
        ui->_comPortComboBox->insertItem(i, _comPorts[i].c_str());
    }
}

const TgtSerialIntf::TgtConnection OpenDialog::getSerialConfig() const
{
    TgtSerialIntf::TgtConnection ret(ui->_comPortComboBox->currentText().toUtf8().constData(), 115200, 0, 0, 0);
    return ret;
}

OpenDialog::~OpenDialog()
{
    delete ui;
}

void OpenDialog::detectComPorts()
{
    std::stringstream portName;
    boost::asio::io_service ioService;
    boost::system::error_code ec;
    _comPorts.clear();
    for (int i = 0; i < 16; i++)
    {
        portName.str("");
        portName<<BASE_PORTNAME<<i;
        boost::asio::serial_port port(ioService);
        port.open(portName.str(), ec);
        if (port.is_open() == true)
        {
            _comPorts.push_back(portName.str());
        }
        else
        {
            qDebug("%s: %s", ec.message().c_str(), portName.str().c_str());
        }
    }
}
