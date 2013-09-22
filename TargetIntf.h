
#ifndef CJD_TARGETINTF_H
#define CJD_TARGETINTF_H

#include <list>
#include <fstream>
#include <string>
#ifndef Q_MOC_RUN
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#endif

class TgtIntf
{
public:
    TgtIntf(void);
    virtual ~TgtIntf(void);

    virtual void TgtConnect()
    {
        TgtMakeConnection();
    };
    virtual int TgtDisconnect() = 0;
    virtual int TgtRead(boost::asio::mutable_buffer b) = 0;
    virtual int TgtWrite(char *szWriteData, int nBytes) = 0;
    virtual bool TgtConnected() = 0;
    virtual void TgtGetTitle(std::string *szTitle) = 0;
    virtual int TgtGetBytesRx() { return m_nTotalRx; };
    virtual int TgtGetBytesTx() { return m_nTotalTx; };
protected:
    virtual void TgtMakeConnection() = 0;
    int m_nTotalTx;
    int m_nTotalRx;
    std::list<boost::asio::mutable_buffer> _incomingData;
    std::vector<boost::asio::mutable_buffer> _outgoingData;
    boost::array<char, 4096> _buffer;
};

enum eTelnetState
{
    TELNET_STATE_DATA,
    TELNET_STATE_COMMAND,
    TELNET_STATE_OPTION
};

enum eTelnetCommand
{
    TELNET_CMD_SE     = 0xf0,   /*End of subnegotiation parameters*/
    TELNET_CMD_NOP    = 0xf1,   /*No operation*/
    TELNET_CMD_DM     = 0xf2,   /*Data mark*/
    TELNET_CMD_BRK    = 0xf3,   /*Break*/
    TELNET_CMD_IP     = 0xf4,   /*Suspend*/
    TELNET_CMD_AO     = 0xf5,   /*Abort output*/
    TELNET_CMD_AYT    = 0xf6,   /*Are you there*/
    TELNET_CMD_EC     = 0xf7,   /*Erase character*/
    TELNET_CMD_EL     = 0xf8,   /*Erase line*/
    TELNET_CMD_GA     = 0xf9,   /*Go ahead*/
    TELNET_CMD_SB     = 0xfa,   /*Subnegotiation*/
    TELNET_CMD_WILL   = 0xfb,   /*will*/
    TELNET_CMD_WONT   = 0xfc,   /*wont*/
    TELNET_CMD_DO     = 0xfd,   /*do*/
    TELNET_CMD_DONT   = 0xfe,   /*dont*/
    TELNET_CMD_IAC    = 0xff    /*Interpret as command*/
};

enum eTelnetOption
{
    TELNET_OPT_BIN    = 0x00,    /* Binary Transmission */
    TELNET_OPT_ECHO   = 0x01,    /* Echo */
    TELNET_OPT_RECN   = 0x02,    /* Reconnection */
    TELNET_OPT_SUPP   = 0x03,    /* Suppress Go Ahead */
    TELNET_OPT_APRX   = 0x04,    /* Approx Message Size Negotiation */
    TELNET_OPT_STAT   = 0x05,    /* Status */
    TELNET_OPT_TIM    = 0x06,    /* Timing Mark */
    TELNET_OPT_REM    = 0x07,    /* Remote Controlled Trans and Echo */
    TELNET_OPT_OLW    = 0x08,    /* Output Line Width */
    TELNET_OPT_OPS    = 0x09,    /* Output Page Size */
    TELNET_OPT_OCRD   = 0x0a,   /* Output Carriage-Return Disposition */
    TELNET_OPT_OHT    = 0x0b,   /* Output Horizontal Tabstops */
    TELNET_OPT_OHTD   = 0x0c,   /* Output Horizontal Tab Disposition */
    TELNET_OPT_OFD    = 0x0d,   /* Output Formfeed Disposition */
    TELNET_OPT_OVT    = 0x0e,   /* Output Vertical Tabstops */
    TELNET_OPT_OVTD   = 0x0f,   /* Output Vertical Tab Disposition */
    TELNET_OPT_OLD    = 0x10,   /* Output Linefeed Disposition */
    TELNET_OPT_EXT    = 0x11,   /* Extended ASCII */
    TELNET_OPT_LOGO   = 0x12,   /* Logout */
    TELNET_OPT_BYTE   = 0x13,   /* Byte Macro */
    TELNET_OPT_DATA   = 0x14,   /* Data Entry Terminal */
    TELNET_OPT_SUP    = 0x15,   /* SUPDUP */
    TELNET_OPT_SUPO   = 0x16,   /* SUPDUP Output */
    TELNET_OPT_SNDL   = 0x17,   /* Send Location */
    TELNET_OPT_TERM   = 0x18,   /* Terminal Type */
    TELNET_OPT_EOR    = 0x19,   /* End of Record */
    TELNET_OPT_TACACS = 0x1a,   /* TACACS User Identification */
    TELNET_OPT_OM     = 0x1b,   /* Output Marking */
    TELNET_OPT_TLN    = 0x1c,   /* Terminal Location Number */
    TELNET_OPT_3270   = 0x1d,   /* Telnet 3270 Regime */
    TELNET_OPT_X3     = 0x1e,   /* X.3 PAD */
    TELNET_OPT_NAWS   = 0x1f,   /* Negotiate About Window Size */
    TELNET_OPT_TS     = 0x20,   /* Terminal Speed */
    TELNET_OPT_RFC    = 0x21,   /* Remote Flow Control */
    TELNET_OPT_LINE   = 0x22,   /* Linemode */
    TELNET_OPT_XDL    = 0x23,   /* X Display Location */
    TELNET_OPT_ENVIR  = 0x24,   /* Telnet Environment Option */
    TELNET_OPT_AUTH   = 0x25,   /* Telnet Authentication Option */
    TELNET_OPT_NENVIR = 0x27,   /* Telnet Environment Option */
    TELNET_OPT_EXTOP  = 0xff,   /* Extended-Options-List */
};

class TgtTelnetIntf : public TgtIntf
{
public:
    TgtTelnetIntf();
    virtual ~TgtTelnetIntf();
    virtual int TgtDisconnect();
    virtual int TgtRead(char *szReadData, int nMaxBytes);
    virtual int TgtWrite(char *szWriteData, int nBytes);
    virtual bool TgtConnected();
    virtual void TgtGetTitle(char *szTitle);
    virtual void TgtSetConfig(const std::string &szServerName, const int nPort, const std::string &szDescription)
    {
        m_sTgtConnection.m_szServerName = szServerName;
        m_sTgtConnection.m_szDescription = szDescription;
        m_sTgtConnection.m_nPort = nPort;
    };
    struct TgtConnection
    {
        std::string m_szServerName;
        int m_nPort;
        std::string m_szDescription;
    };

protected:
    int TgtTelnet(char *sTelnetRx, int nNumBytes, char *szReadData);
    int TgtTelnetData(unsigned char cTelnetRx, char* cReadData);
    int TgtTelnetCommand(eTelnetCommand cTelnetRx);
    int TgtTelnetOption(eTelnetOption eOpt);
    int TgtProcessEcho();
    int TgtProcessTerm();
    int TgtProcessUnknownOption(eTelnetOption eOpt);
    int TgtDeny(eTelnetOption eOpt);
    int TgtConfirm(eTelnetOption eOpt);
    int TgtSendCommand(eTelnetCommand eCmd, eTelnetOption eOpt);
    virtual void TgtMakeConnection();

    int m_nSocket;
    char m_sTelnetRx[512];
    bool m_bEcho;
    eTelnetCommand m_nCommand;
    eTelnetState m_nState;
    TgtConnection m_sTgtConnection;
    bool m_bConnected;
};

class TgtSerialIntf : public TgtIntf
{
public:
    struct TgtConnection
    {
        TgtConnection(const std::string &szPortName,
                      const boost::asio::serial_port_base::baud_rate baudRate,
                      const boost::asio::serial_port_base::parity parity,
                      const boost::asio::serial_port_base::stop_bits stopBits,
                      const boost::asio::serial_port_base::character_size byteSize,
                      const boost::asio::serial_port_base::flow_control flowControl)
            : _portName(szPortName),
              _baudRate(baudRate),
              _parity(parity),
              _stopBits(stopBits),
              _byteSize(byteSize),
              _flowControl(flowControl)
        {
        }

        std::string _portName;
        boost::asio::serial_port_base::baud_rate _baudRate;
        boost::asio::serial_port_base::parity _parity;
        boost::asio::serial_port_base::stop_bits _stopBits;
        boost::asio::serial_port_base::character_size _byteSize;
        boost::asio::serial_port_base::flow_control _flowControl;
    };
    static boost::shared_ptr<TgtSerialIntf> createSerialConnection(const TgtConnection &config);
    virtual ~TgtSerialIntf ();
    virtual int TgtDisconnect();
    virtual int TgtRead(boost::asio::mutable_buffer b);
    virtual int TgtWrite(char *szWriteData, int nBytes);
    virtual bool TgtConnected();
    virtual void TgtGetTitle(std::string *szTitle);
    virtual TgtConnection TgtGetConfig()
    {
        return _tgtConnectionConfig;
    }

protected:
    TgtSerialIntf (const TgtConnection &config);
    virtual void TgtMakeConnection();
    virtual void TgtReadFromPort();
    virtual void TgtSendToPort();
    virtual void TgtWritePortData(char *szData, int nBytes);
    void TgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred);

    TgtConnection _tgtConnectionConfig;
    boost::asio::io_service _service;
    boost::asio::serial_port _port;
    boost::scoped_ptr<boost::thread> _serialThread;
};


class TgtFileIntf : public TgtIntf
{
public:
    struct TgtConnection
    {
        TgtConnection(std::string fileName)
            :_fileName(fileName)
        {
        }
        std::string _fileName;
    };

    static boost::shared_ptr<TgtFileIntf> createFileConnection(const TgtConnection &config);
    virtual ~TgtFileIntf(void);

    virtual int TgtDisconnect();
    virtual int TgtRead(boost::asio::mutable_buffer b);
    virtual int TgtWrite(char *szWriteData, int nBytes);
    virtual bool TgtConnected();
    virtual void TgtGetTitle(std::string *szTitle);

protected:
    TgtFileIntf(const TgtConnection &config);
    virtual void TgtMakeConnection();

    std::ifstream _inputFile;
    TgtConnection _tgtConnectionConfig;
};
#endif
