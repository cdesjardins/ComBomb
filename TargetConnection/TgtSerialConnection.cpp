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
#include "TgtSerialConnection.h"
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#endif

/******************************************************************************
**
**  Serial
**
******************************************************************************/

std::shared_ptr<TgtSerialIntf> TgtSerialIntf::createSerialConnection(
    const std::shared_ptr<const TgtConnectionConfig>& config)
{
    std::shared_ptr<TgtSerialIntf> ret(new TgtSerialIntf(config));
    ret->tgtAttemptReconnect();
    return ret;
}

void TgtSerialIntf::platformSerialConfig()
{
#ifndef WIN32
    struct termios tty;
    tcgetattr(_port->native_handle(), &tty);
    tty.c_cc[VTIME] = 5;
    tty.c_cc[VMIN] = 1;
    tty.c_iflag =  IGNBRK;
    tcsetattr(_port->native_handle(), TCSANOW, &tty);
#endif
}

void TgtSerialIntf::tgtMakeConnection()
{
    std::shared_ptr<const TgtConnectionConfig> connectionConfig = std::dynamic_pointer_cast<const TgtConnectionConfig>(
        _connectionConfig);
    //_service.reset(new boost::asio::io_service());
    _port.reset(new boost::asio::serial_port(_service, connectionConfig->_portName));
    _port->set_option(connectionConfig->_baudRate);
    _port->set_option(connectionConfig->_parity);
    _port->set_option(connectionConfig->_byteSize);
    _port->set_option(connectionConfig->_stopBits);
    _port->set_option(connectionConfig->_flowControl);
    platformSerialConfig();
    boost::system::error_code err;

    _serialWriterThread = TgtThread::create(boost::protect(std::bind(&TgtSerialIntf::writerThread, this)));
    _serialServiceThread = TgtThread::create(boost::protect(std::bind(&TgtSerialIntf::serviceThread, this)));
    _bufferPool->dequeue(_currentIncomingBuffer);
    tgtReadCallback(err, 0);
}

bool TgtSerialIntf::serviceThread()
{
    _service.reset();
    if (_service.poll() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return true;
}

bool TgtSerialIntf::writerThread()
{
    boost::intrusive_ptr<RefCntBuffer> b;
    boost::system::error_code ec;
    bool attemptReconnect = false;
    if (_outgoingData.dequeue(b, 100) == true)
    {
        boost::asio::write(*_port.get(), boost::asio::buffer(b->_buffer), ec);
        if (ec)
        {
            attemptReconnect = true;
        }
    }
    if (attemptReconnect == true)
    {
        tgtAttemptReconnect();
    }
    return !attemptReconnect;
}

TgtSerialIntf::TgtSerialIntf(const std::shared_ptr<const TgtConnectionConfig>& config)
    : TgtIntf(config)
{
}

TgtSerialIntf::~TgtSerialIntf ()
{
    tgtDisconnect();
}

void TgtSerialIntf::tgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (!error)
    {
        boost::asio::mutable_buffer buffer;

        if (bytesTransferred > 0)
        {
            if (_currentIncomingBuffer != nullptr)
            {
                _currentIncomingBuffer->_buffer =
                    boost::asio::buffer(_currentIncomingBuffer->_buffer, bytesTransferred);
                _incomingData.enqueue(_currentIncomingBuffer);
            }
            _bufferPool->dequeue(_currentIncomingBuffer, 100);
        }
        if (_currentIncomingBuffer == nullptr)
        {
            // If there are no buffers available then just throw away the next
            // bit if incoming data...
            buffer = boost::asio::buffer(_throwAway, sizeof(_throwAway) - 1);
        }
        else
        {
            buffer = _currentIncomingBuffer->_buffer;
        }
        _port->async_read_some(boost::asio::buffer(buffer, boost::asio::buffer_size(buffer) - 1),
                               boost::bind(&TgtSerialIntf::tgtReadCallback, this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        tgtAttemptReconnect();
    }
}

void TgtSerialIntf::tgtBreakConnection()
{
    _serialWriterThread.reset();
    _serialServiceThread.reset();
    if (_port != nullptr)
    {
        if (_port->is_open())
        {
            boost::system::error_code ec;
            _port->close(ec);
        }
        _port.reset();
    }
}

void TgtSerialIntf::tgtGetTitle(std::string* szTitle)
{
    std::string parity;
    std::string stopbits;
    std::shared_ptr<const TgtConnectionConfig> connectionConfig = std::dynamic_pointer_cast<const TgtConnectionConfig>(
        _connectionConfig);

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
