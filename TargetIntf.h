
#ifndef TARGETINTF_H
#define TARGETINTF_H

#include <list>

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
    virtual int TgtRead(char *szReadData, int nMaxBytes) = 0;
    virtual int TgtWrite(char *szWriteData, int nBytes) = 0;
    virtual bool TgtConnected() = 0;
    virtual void TgtGetTitle(std::string *szTitle) = 0;
    virtual int TgtGetBytesRx() { return m_nTotalRx; };
    virtual int TgtGetBytesTx() { return m_nTotalTx; };
protected:
    virtual void TgtMakeConnection() = 0;
    int m_nTotalTx;
    int m_nTotalRx;
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
    virtual void TgtSetConfig(char *szServerName, int nPort, char *szDescription)
    {
        strncpy(m_sTgtConnection.m_szDescription, szDescription, sizeof(m_sTgtConnection.m_szDescription));
        strncpy(m_sTgtConnection.m_szServerName, szServerName, sizeof(m_sTgtConnection.m_szServerName));
        m_sTgtConnection.m_nPort = nPort;
    };
    struct TgtConnection
    {
        char m_szServerName[256];
        int m_nPort;
        char m_szDescription[256];
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
        char m_szPortName[256];
        unsigned long m_dwBaudRate;
        unsigned char m_byParity;
        unsigned char m_byStopBits;
        unsigned char m_byByteSize;
    };

    TgtSerialIntf ();
    virtual ~TgtSerialIntf ();
    virtual int TgtDisconnect();
    virtual int TgtRead(char *szReadData, int nMaxBytes);
    virtual int TgtWrite(char *szWriteData, int nBytes);
    virtual bool TgtConnected();
    virtual void TgtGetTitle(char *szTitle);
    virtual char * TgtSetupPort();
    virtual TgtConnection TgtGetConfig()
    {
        return m_sTgtConnection;
    };
    virtual void TgtSetConfig(TgtConnection *pTgtConfig)
    {
        memcpy(&m_sTgtConnection, pTgtConfig, sizeof(TgtConnection));
    };
    virtual void TgtSetConfig(char *szPortName,
        unsigned long dwBaudRate,
        unsigned char byParity,
        unsigned char byStopBits,
        unsigned char byByteSize)
    {
        strncpy(m_sTgtConnection.m_szPortName, szPortName, sizeof(m_sTgtConnection.m_szPortName));
        m_sTgtConnection.m_dwBaudRate = dwBaudRate;
        m_sTgtConnection.m_byParity   = byParity;
        m_sTgtConnection.m_byStopBits = byStopBits;
        m_sTgtConnection.m_byByteSize = byByteSize;
    };


protected:
    virtual void TgtMakeConnection();
    static void TgtSerialMonitor(void *arg);
    virtual void TgtReadFromPort();
    virtual void TgtSendToPort();
    virtual void TgtWritePortData(char *szData, int nBytes);
/*
    HANDLE m_hSerialMonitor;
    HANDLE m_hThreadTerm;
    HANDLE m_hSerial;
    HANDLE m_hOutput;
    std::list<char> m_dataIncoming;
    std::list<char> m_dataOutgoing;
    CRITICAL_SECTION m_csInProtector;
    CRITICAL_SECTION m_csOutProtector;
*/
    TgtConnection m_sTgtConnection;
};


class TgtFileIntf : public TgtIntf
{
public:
    TgtFileIntf(void);
    virtual ~TgtFileIntf(void);

    virtual int TgtDisconnect();
    virtual int TgtRead(char *szReadData, int nMaxBytes);
    virtual int TgtWrite(char *szWriteData, int nBytes);
    virtual bool TgtConnected();
    virtual void TgtGetTitle(std::string *szTitle);
    virtual void TgtSetConfig(const std::string &szFileName)
    {
        m_sTgtConnection.m_szFileName = szFileName;
    };
    struct TgtConnection
    {
        std::string m_szFileName;
    };
protected:
    virtual void TgtMakeConnection();

    FILE *m_fInput;
    int m_cnt;
    TgtConnection m_sTgtConnection;
};
#endif
