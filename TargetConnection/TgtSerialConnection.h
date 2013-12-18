#ifndef CB_TGT_SERIAL_CONNECTION_H
#define CB_TGT_SERIAL_CONNECTION_H

#include "QTerminal/TgtIntf.h"
#ifndef Q_MOC_RUN
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#endif

class TgtSerialIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(const std::string &szPortName, const boost::asio::serial_port_base::baud_rate baudRate, const boost::asio::serial_port_base::parity parity, const boost::asio::serial_port_base::stop_bits stopBits, const boost::asio::serial_port_base::character_size byteSize, const boost::asio::serial_port_base::flow_control flowControl)
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
    static boost::shared_ptr<TgtSerialIntf> createSerialConnection(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual ~TgtSerialIntf ();
    virtual int tgtBreakConnection(bool joinWriter);
    virtual int tgtBreakConnection()
    {
        return tgtBreakConnection(true);
    }

    virtual bool tgtConnected();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    TgtSerialIntf (const boost::shared_ptr<const TgtConnectionConfig> &config);
    void tgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred);
    void serviceThread();
    void writerThread();
    virtual void tgtMakeConnection();
    void tgtStopService();

    boost::asio::io_service _service;
    boost::scoped_ptr<boost::asio::serial_port> _port;
    boost::scoped_ptr<boost::thread> _serialServiceThread;
    boost::scoped_ptr<boost::thread> _serialWriterThread;
    volatile bool _serialWriterThreadRun;
    volatile bool _serialServiceThreadRun;
};



#endif
