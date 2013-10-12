#include "TgtSerialConnection.h"
/******************************************************************************
**
**  Serial
**
******************************************************************************/

boost::shared_ptr<TgtSerialIntf> TgtSerialIntf::createSerialConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtSerialIntf> ret(new TgtSerialIntf(config));
    ret->TgtMakeConnection();
    return ret;
}

void TgtSerialIntf::TgtMakeConnection()
{
    _serialThreadsRun = true;
    _serialServiceThread.reset(new boost::thread(boost::bind(&TgtSerialIntf::serviceThread, this)));
    _serialWriterThread.reset(new  boost::thread(boost::bind(&TgtSerialIntf::writerThread, this)));

    boost::system::error_code err;
    TgtReadCallback(err, 0);
}

void TgtSerialIntf::serviceThread()
{
    do
    {
        _service.run();
        _service.reset();
    } while (_serialThreadsRun == true);
}

void TgtSerialIntf::writerThread()
{
    boost::asio::mutable_buffer b;
    boost::system::error_code  ec;
    bool attemptReconnect = false;
    while (_serialThreadsRun == true)
    {
        if (_outgoingData.dequeue(b) == true)
        {
            //int ret = _port.write_some(boost::asio::buffer(b));
            boost::asio::write(_port, boost::asio::buffer(b), ec);
            TgtReturnReadBuffer(b);
            if (ec)
            {
                TgtDisconnect();
                attemptReconnect = true;
                break;
            }
        }
        else
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }
    }
    qDebug("serial done");
    if (attemptReconnect == true)
    {
        qDebug("attempt reconnect");
        TgtAttemptReconnect();
    }
}

TgtSerialIntf::TgtSerialIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config),
    _port(_service, config->_portName)
{
    _port.set_option(config->_baudRate);
    _port.set_option(config->_parity);
    _port.set_option(config->_byteSize);
    _port.set_option(config->_stopBits);
    _port.set_option(config->_flowControl);
}

TgtSerialIntf::~TgtSerialIntf ()
{
    tgtDisconnect();
}

void TgtSerialIntf::TgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (!error)
    {
        if (bytesTransferred > 0)
        {
            _incomingData.enqueue(boost::asio::buffer(_currentIncomingBuffer, bytesTransferred));
            _bufferPool.dequeue(_currentIncomingBuffer);
        }
        _port.async_read_some(boost::asio::buffer(_currentIncomingBuffer),
                              boost::bind(&TgtSerialIntf::TgtReadCallback, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
    }
}

int TgtSerialIntf::TgtDisconnect()
{
    _service.stop();
    if (_serialThreadsRun == true)
    {
        _serialThreadsRun = false;
        _serialServiceThread->join();
        _serialWriterThread->join();
    }
    _port.cancel();
    _port.close();
    _service.reset();
    return 0;
}

bool TgtSerialIntf::TgtConnected()
{
    return true;
}

void TgtSerialIntf::TgtGetTitle(std::string* szTitle)
{
    std::string parity;
    std::string stopbits;
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);

    switch (connectionConfig->_parity.value())
    {
        case boost::asio::serial_port_base::parity::even:
            parity = "e";
            break;
        case boost::asio::serial_port_base::parity::none:
            parity = "n";
            break;
        case boost::asio::serial_port_base::parity::odd:
            parity = "o";
            break;
    }
    switch (connectionConfig->_stopBits.value())
    {
        case boost::asio::serial_port_base::stop_bits::one:
            stopbits = "1";
            break;
        case boost::asio::serial_port_base::stop_bits::onepointfive:
            stopbits = "1.5";
            break;
        case boost::asio::serial_port_base::stop_bits::two:
            stopbits = "2";
            break;
    }

    std::stringstream t;
    t << connectionConfig->_portName << " "
      << connectionConfig->_baudRate.value() << " "
      << parity
      << connectionConfig->_byteSize.value()
      << stopbits;
    *szTitle = t.str();
}

