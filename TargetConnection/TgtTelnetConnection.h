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
#ifndef CB_TGT_TELNET_CONNECTION_H
#define CB_TGT_TELNET_CONNECTION_H

#include "QTerminal/TgtIntf.h"
#include "TgtThread.h"
#include <boost/asio.hpp>

enum eTelnetState
{
    TELNET_STATE_DATA,
    TELNET_STATE_COMMAND,
    TELNET_STATE_OPTION
};

enum eTelnetCommand
{
    TELNET_CMD_SE     = 0xf0, /*End of subnegotiation parameters*/
    TELNET_CMD_NOP    = 0xf1, /*No operation*/
    TELNET_CMD_DM     = 0xf2, /*Data mark*/
    TELNET_CMD_BRK    = 0xf3, /*Break*/
    TELNET_CMD_IP     = 0xf4, /*Suspend*/
    TELNET_CMD_AO     = 0xf5, /*Abort output*/
    TELNET_CMD_AYT    = 0xf6, /*Are you there*/
    TELNET_CMD_EC     = 0xf7, /*Erase character*/
    TELNET_CMD_EL     = 0xf8, /*Erase line*/
    TELNET_CMD_GA     = 0xf9, /*Go ahead*/
    TELNET_CMD_SB     = 0xfa, /*Subnegotiation*/
    TELNET_CMD_WILL   = 0xfb, /*will*/
    TELNET_CMD_WONT   = 0xfc, /*wont*/
    TELNET_CMD_DO     = 0xfd, /*do*/
    TELNET_CMD_DONT   = 0xfe, /*dont*/
    TELNET_CMD_IAC    = 0xff /*Interpret as command*/
};

enum eTelnetOption
{
    TELNET_OPT_BIN    = 0x00, /* Binary Transmission */
    TELNET_OPT_ECHO   = 0x01, /* Echo */
    TELNET_OPT_RECN   = 0x02, /* Reconnection */
    TELNET_OPT_SUPP   = 0x03, /* Suppress Go Ahead */
    TELNET_OPT_APRX   = 0x04, /* Approx Message Size Negotiation */
    TELNET_OPT_STAT   = 0x05, /* Status */
    TELNET_OPT_TIM    = 0x06, /* Timing Mark */
    TELNET_OPT_REM    = 0x07, /* Remote Controlled Trans and Echo */
    TELNET_OPT_OLW    = 0x08, /* Output Line Width */
    TELNET_OPT_OPS    = 0x09, /* Output Page Size */
    TELNET_OPT_OCRD   = 0x0a, /* Output Carriage-Return Disposition */
    TELNET_OPT_OHT    = 0x0b, /* Output Horizontal Tabstops */
    TELNET_OPT_OHTD   = 0x0c, /* Output Horizontal Tab Disposition */
    TELNET_OPT_OFD    = 0x0d, /* Output Formfeed Disposition */
    TELNET_OPT_OVT    = 0x0e, /* Output Vertical Tabstops */
    TELNET_OPT_OVTD   = 0x0f, /* Output Vertical Tab Disposition */
    TELNET_OPT_OLD    = 0x10, /* Output Linefeed Disposition */
    TELNET_OPT_EXT    = 0x11, /* Extended ASCII */
    TELNET_OPT_LOGO   = 0x12, /* Logout */
    TELNET_OPT_BYTE   = 0x13, /* Byte Macro */
    TELNET_OPT_DATA   = 0x14, /* Data Entry Terminal */
    TELNET_OPT_SUP    = 0x15, /* SUPDUP */
    TELNET_OPT_SUPO   = 0x16, /* SUPDUP Output */
    TELNET_OPT_SNDL   = 0x17, /* Send Location */
    TELNET_OPT_TERM   = 0x18, /* Terminal Type */
    TELNET_OPT_EOR    = 0x19, /* End of Record */
    TELNET_OPT_TACACS = 0x1a, /* TACACS User Identification */
    TELNET_OPT_OM     = 0x1b, /* Output Marking */
    TELNET_OPT_TLN    = 0x1c, /* Terminal Location Number */
    TELNET_OPT_3270   = 0x1d, /* Telnet 3270 Regime */
    TELNET_OPT_X3     = 0x1e, /* X.3 PAD */
    TELNET_OPT_NAWS   = 0x1f, /* Negotiate About Window Size */
    TELNET_OPT_TS     = 0x20, /* Terminal Speed */
    TELNET_OPT_RFC    = 0x21, /* Remote Flow Control */
    TELNET_OPT_LINE   = 0x22, /* Linemode */
    TELNET_OPT_XDL    = 0x23, /* X Display Location */
    TELNET_OPT_ENVIR  = 0x24, /* Telnet Environment Option */
    TELNET_OPT_AUTH   = 0x25, /* Telnet Authentication Option */
    TELNET_OPT_NENVIR = 0x27, /* Telnet Environment Option */
    TELNET_OPT_EXTOP  = 0xff, /* Extended-Options-List */
};

class TgtTelnetIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(const std::string &hostName, const int portNum)
            : _hostName(hostName),
            _portNum(portNum)
        {
        }

        TgtConnectionConfig()
        {
        }

        std::string    _hostName;
        unsigned short _portNum;
    };

    static boost::shared_ptr<TgtTelnetIntf> createTelnetConnection(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual ~TgtTelnetIntf();
    virtual bool tgtConnected();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    TgtTelnetIntf(const boost::shared_ptr<const TgtConnectionConfig> &config);
    int tgtTelnetProcessData(const boost::intrusive_ptr<RefCntBuffer> &readData);
    int tgtTelnetData(unsigned char cTelnetRx, char* cReadData);
    int tgtTelnetCommand(eTelnetCommand cTelnetRx);
    int tgtTelnetOption(eTelnetOption eOpt);
    int tgtProcessEcho();
    int tgtProcessTerm();
    int tgtProcessUnknownOption(eTelnetOption eOpt);
    int tgtDeny(eTelnetOption eOpt);
    int tgtConfirm(eTelnetOption eOpt);
    void tgtSendCommand(eTelnetCommand eCmd, eTelnetOption eOpt);
    virtual void tgtMakeConnection();
    void tgtSendData(const boost::asio::mutable_buffer &buf);
    bool serviceThread();
    bool writerThread();
    void tgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred);
    virtual void tgtBreakConnection();

    boost::scoped_ptr<boost::asio::ip::tcp::socket> _socket;
    boost::asio::io_service _socketService;

    bool m_bEcho;
    eTelnetCommand m_nCommand;
    eTelnetState m_nState;
    bool m_bConnected;
    boost::shared_ptr<TgtThread> _telnetWriterThread;
    boost::shared_ptr<TgtThread> _telnetServiceThread;
    boost::intrusive_ptr<RefCntBuffer> _currentIncomingBuffer;
    char _throwAway[1024];
};

#endif
