#ifndef TGTINTF_H
#define TGTINTF_H

#define CB_DEFAULT_TERM_WIDTH 80
#define CB_DEFAULT_TERM_HEIGHT 25

#ifndef Q_MOC_RUN
#include <boost/asio/buffer.hpp>
#include <boost/smart_ptr.hpp>
#endif
#include "impl/ThreadSafeQueue.h"
#include "impl/garbagecollector.h"
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

    int tgtDisconnect(bool running = false);
    virtual int tgtRead(boost::shared_ptr<boost::asio::mutable_buffer> &b);
    virtual int tgtWrite(const char* szWriteData, int nBytes);
    virtual bool tgtConnected() = 0;
    virtual void tgtGetTitle(std::string* szTitle) = 0;
    virtual int tgtGetBytesRx()
    {
        return m_nTotalRx;
    };
    virtual int tgtGetBytesTx()
    {
        return m_nTotalTx;
    };
    void tgtReturnReadBuffer(const boost::shared_ptr<boost::asio::mutable_buffer> &b);
    boost::shared_ptr<const TgtConnectionConfigBase> getConfig()
    {
        return _connectionConfig;
    }

signals:
    void updateStatusSignal(QString);
    void updateTitleSignal(QString);
protected:
    void tgtAttemptReconnect();
    virtual void tgtMakeConnection() = 0;
    virtual int tgtBreakConnection() = 0;

    int m_nTotalTx;
    int m_nTotalRx;
    ThreadSafeQueue<boost::shared_ptr<boost::asio::mutable_buffer> > _incomingData;
    ThreadSafeQueue<boost::shared_ptr<boost::asio::mutable_buffer> > _outgoingData;
    ThreadSafeQueue<boost::shared_ptr<boost::asio::mutable_buffer> > _bufferPool;
    boost::shared_ptr<boost::asio::mutable_buffer> _currentIncomingBuffer;
    boost::shared_ptr<const TgtConnectionConfigBase> _connectionConfig;
private:
    bool _running;
    static int deleteBuffersFunctor(std::list<boost::shared_ptr<boost::asio::mutable_buffer> > &pool);
    boost::mutex _disconnectMutex;
    boost::shared_ptr<GarbageCollector<boost::asio::mutable_buffer> > _garbageCollector;
#ifdef CB_TRAP_TO_FILE
    std::ofstream _trapFile;
#endif
};


#endif // TGTINTF_H
