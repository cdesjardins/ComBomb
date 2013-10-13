#ifndef TGTINTF_H
#define TGTINTF_H

#define CB_DEFAULT_TERM_WIDTH 80
#define CB_DEFAULT_TERM_HEIGHT 25

#ifndef Q_MOC_RUN
#include <boost/asio/buffer.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif
#include "impl/ThreadSafeQueue.h"
#include "../unparam.h"
#include <QWidget>
#include <fstream>
//#define CB_TRAP_TO_FILE

class TgtIntf : public QWidget
{
    Q_OBJECT
public:
    struct TgtConnectionConfigBase
    {
        virtual ~TgtConnectionConfigBase()
        {
        }
    };

    TgtIntf(const boost::shared_ptr<const TgtConnectionConfigBase> &config);
    virtual ~TgtIntf(void);

    int tgtDisconnect();
    virtual int TgtRead(boost::asio::mutable_buffer &b);
    virtual int TgtWrite(const char* szWriteData, int nBytes);
    virtual bool TgtConnected() = 0;
    virtual void TgtGetTitle(std::string* szTitle) = 0;
    virtual int TgtGetBytesRx()
    {
        return m_nTotalRx;
    };
    virtual int TgtGetBytesTx()
    {
        return m_nTotalTx;
    };
    void TgtReturnReadBuffer(const boost::asio::mutable_buffer &b);
    boost::shared_ptr<const TgtConnectionConfigBase> getConfig()
    {
        return _connectionConfig;
    }

signals:
    void updateStatusSignal(QString);
    void updateTitleSignal(QString);
protected:
    void TgtAttemptReconnect();
    virtual void TgtMakeConnection() = 0;
    virtual int TgtDisconnect() = 0;

    int m_nTotalTx;
    int m_nTotalRx;
    ThreadSafeQueue<boost::asio::mutable_buffer> _incomingData;
    ThreadSafeQueue<boost::asio::mutable_buffer> _outgoingData;
    ThreadSafeQueue<boost::asio::mutable_buffer> _bufferPool;
    boost::asio::mutable_buffer _currentIncomingBuffer;
    boost::shared_ptr<const TgtConnectionConfigBase> _connectionConfig;
private:
    bool _running;
    static int deleteBuffersFunctor(std::list<boost::asio::mutable_buffer> &pool);
    boost::mutex _disconnectMutex;
#ifdef CB_TRAP_TO_FILE
    std::ofstream _trapFile;
#endif
};

namespace boost
{
namespace serialization
{
template<class Archive>
void serialize(Archive & ar, TgtIntf::TgtConnectionConfigBase & configbase, const unsigned int version)
{
    UNREF_PARAM(ar);
    UNREF_PARAM(version);
    UNREF_PARAM(configbase);
}
}
}

#endif // TGTINTF_H
