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
#ifndef CB_TGT_SERIAL_CONNECTION_H
#define CB_TGT_SERIAL_CONNECTION_H

#include "../QTerminal/TgtIntf.h"
#include "TgtThread.h"
#include <boost/asio.hpp>

class TgtSerialIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(const std::string& szPortName, const boost::asio::serial_port_base::baud_rate baudRate, const boost::asio::serial_port_base::parity parity, const boost::asio::serial_port_base::stop_bits stopBits, const boost::asio::serial_port_base::character_size byteSize, const boost::asio::serial_port_base::flow_control flowControl)
            : _portName(szPortName),
            _baudRate(baudRate),
            _parity(parity),
            _stopBits(stopBits),
            _byteSize(byteSize),
            _flowControl(flowControl)
        {
        }

        std::string _portName;
        boost::asio::serial_port_base::baud_rate      _baudRate;
        boost::asio::serial_port_base::parity         _parity;
        boost::asio::serial_port_base::stop_bits      _stopBits;
        boost::asio::serial_port_base::character_size _byteSize;
        boost::asio::serial_port_base::flow_control   _flowControl;
    };
    static std::shared_ptr<TgtSerialIntf> createSerialConnection(const std::shared_ptr<const TgtConnectionConfig>& config);
    virtual ~TgtSerialIntf ();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    TgtSerialIntf (const std::shared_ptr<const TgtConnectionConfig>& config);
    void tgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred);
    bool serviceThread();
    bool writerThread();
    virtual void tgtBreakConnection();
    virtual void tgtMakeConnection();

    boost::asio::io_service _service;
    std::unique_ptr<boost::asio::serial_port> _port;
    std::shared_ptr<TgtThread> _serialServiceThread;
    std::shared_ptr<TgtThread> _serialWriterThread;
    boost::intrusive_ptr<RefCntBuffer> _currentIncomingBuffer;
    char _throwAway[1024];
};

#endif
