#include "TgtSerialConnection.h"
/******************************************************************************
**
**  Serial
**
******************************************************************************/

boost::shared_ptr<TgtSerialIntf> TgtSerialIntf::createSerialConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtSerialIntf> ret(new TgtSerialIntf(config));
    ret->tgtMakeConnection();
    return ret;
}

void TgtSerialIntf::tgtMakeConnection()
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
    _serialServiceThreadRun = true;
    _serialServiceThread.reset(new boost::thread(boost::bind(&TgtSerialIntf::serviceThread, this)));
    boost::system::error_code err;
    _bufferPool->dequeue(_currentIncomingBuffer);
    tgtReadCallback(err, 0);
}

void TgtSerialIntf::serviceThread()
{
    do
    {
        _service.reset();
        _service.poll();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
    } while (_serialServiceThreadRun == true);
}

void TgtSerialIntf::writerThread()
{
    boost::intrusive_ptr<RefCntBuffer> b;
    boost::system::error_code ec;
    bool attemptReconnect = false;
    while (_serialWriterThreadRun == true)
    {
        if (_outgoingData.dequeue(b, 1) == true)
        {
            boost::asio::write(*_port.get(), boost::asio::buffer(b->_buffer), ec);
            if (ec)
            {
                tgtBreakConnection(false);
                attemptReconnect = true;
                break;
            }
        }
    }
    if (attemptReconnect == true)
    {
        tgtAttemptReconnect();
    }
}

TgtSerialIntf::TgtSerialIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config)

{
}

TgtSerialIntf::~TgtSerialIntf ()
{
    tgtDisconnect();
}

void TgtSerialIntf::tgtStopService()
{
    _serialServiceThreadRun = false;
    //_service.stop();
    if ((_serialServiceThread->joinable() == true) && (boost::this_thread::get_id() != _serialServiceThread->get_id()))
    {
        _serialServiceThread->join();
    }
}

void TgtSerialIntf::tgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred)
{

    if (!error)
    {
        boost::asio::mutable_buffer buffer;

        if (bytesTransferred > 0)
        {
            if (_currentIncomingBuffer != NULL)
            {
                _currentIncomingBuffer->_buffer = boost::asio::buffer(_currentIncomingBuffer->_buffer, bytesTransferred);
                _incomingData.enqueue(_currentIncomingBuffer);
            }
            _bufferPool->dequeue(_currentIncomingBuffer);
        }
        if (_currentIncomingBuffer == NULL)
        {
            // If there are no buffers available then just throw away the next
            // bit if incoming data...
            buffer = boost::asio::buffer(_throwAway, sizeof(_throwAway) - 1);
        }
        else
        {
            buffer = _currentIncomingBuffer->_buffer;
        }
        _port->async_read_some(boost::asio::buffer(buffer),
                               boost::bind(&TgtSerialIntf::tgtReadCallback, this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        tgtDisconnect(true);
        tgtAttemptReconnect();
    }
}

int TgtSerialIntf::tgtBreakConnection(bool joinWriter)
{
    tgtStopService();

    if (_serialWriterThreadRun == true)
    {
        _serialWriterThreadRun = false;
        if ((_serialWriterThread->joinable()) && (joinWriter == true))
        {
            _serialWriterThread->join();
        }
    }
    if (_port != NULL)
    {
        if (_port->is_open())
        {
            _port->cancel();
            _port->close();
        }
        _port.reset();
    }
    return 0;
}

bool TgtSerialIntf::tgtConnected()
{
    return true;
}

void TgtSerialIntf::tgtGetTitle(std::string* szTitle)
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

