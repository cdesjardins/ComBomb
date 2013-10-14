#include "TgtSerialConnection.h"
/******************************************************************************
**
**  Serial
**
******************************************************************************/

boost::shared_ptr<TgtSerialIntf> TgtSerialIntf::createSerialConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtSerialIntf> ret(new TgtSerialIntf(config));
    ret->_serialServiceThreadRun = true;
    ret->_serialServiceThread.reset(new boost::thread(boost::bind(&TgtSerialIntf::serviceThread, ret.get())));
    ret->TgtMakeConnection();
    return ret;
}

void TgtSerialIntf::TgtMakeConnection()
{
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    //_service.reset(new boost::asio::io_service());
    _port.reset(new boost::asio::serial_port(_service, connectionConfig->_portName));
    _port->set_option(connectionConfig->_baudRate);
    _port->set_option(connectionConfig->_parity);
    _port->set_option(connectionConfig->_byteSize);
    _port->set_option(connectionConfig->_stopBits);
    _port->set_option(connectionConfig->_flowControl);

    _serialWriterThreadRun = true;
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
    } while (_serialServiceThreadRun == true);
}

void TgtSerialIntf::writerThread()
{
    boost::asio::mutable_buffer b;
    boost::system::error_code  ec;
    bool attemptReconnect = false;
    while (_serialWriterThreadRun == true)
    {
        if (_outgoingData.dequeue(b) == true)
        {
            boost::asio::write(*_port.get(), boost::asio::buffer(b), ec);
            TgtReturnReadBuffer(b);
            if (ec)
            {
                TgtDisconnect(false);
                attemptReconnect = true;
                break;
            }
        }
        else
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }
    }
    if (attemptReconnect == true)
    {
        TgtAttemptReconnect();
    }
}

TgtSerialIntf::TgtSerialIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config)

{
}

TgtSerialIntf::~TgtSerialIntf ()
{
    tgtStopService();
    tgtDisconnect();
}

void TgtSerialIntf::tgtStopService()
{
    _serialServiceThreadRun = false;
    _service.stop();
    if (_serialServiceThread->joinable())
    {
        _serialServiceThread->join();
    }
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
        _port->async_read_some(boost::asio::buffer(_currentIncomingBuffer),
                              boost::bind(&TgtSerialIntf::TgtReadCallback, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        TgtDisconnect();
        TgtAttemptReconnect();
    }
}

int TgtSerialIntf::TgtDisconnect(bool joinWriter)
{
    if (_serialWriterThreadRun == true)
    {
        _serialWriterThreadRun = false;
        if ((_serialWriterThread->joinable()) && (joinWriter == true))
        {
            _serialWriterThread->join();
        }
    }
    _port->cancel();
    _port->close();
    _port.reset();
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

