#include "QTerminal/TgtIntf.h"
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
/*
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/

#define TGT_BUFFER_SIZE   4096
TgtIntf::TgtIntf(const boost::shared_ptr<const TgtConnectionConfigBase> &config)
    : _connectionConfig(config),
    _running(true)

{
#ifdef CB_TRAP_TO_FILE
    std::string trapFileName = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
    _trapFile.open(trapFileName + ".cbd", std::ios::out | std::ios::binary);
#endif
    for (size_t i = 0; i < 4096; i++)
    {
        char* buffer = new char[TGT_BUFFER_SIZE];
        boost::shared_ptr<boost::asio::mutable_buffer> bfrPtr(new boost::asio::mutable_buffer(buffer, TGT_BUFFER_SIZE - 1));
        _bufferPool.enqueue(bfrPtr);
    }

    _bufferPool.dequeue(_currentIncomingBuffer);
    _garbageCollector = GarbageCollector<boost::asio::mutable_buffer>::createGargabageCollector(&_bufferPool);

    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
    _running = false;
    _bufferPool.iterate(boost::bind(&TgtIntf::deleteBuffersFunctor, _1));
#ifdef CB_TRAP_TO_FILE
    _trapFile.close();
#endif
}

int TgtIntf::deleteBuffersFunctor(std::list<boost::shared_ptr<boost::asio::mutable_buffer> > &pool)
{
    int ret = pool.size();
    std::list<boost::shared_ptr<boost::asio::mutable_buffer> >::iterator it;
    for (it = pool.begin(); it != pool.end(); it++)
    {
        boost::shared_ptr<boost::asio::mutable_buffer> bfrPtr = *it;
        char* data = boost::asio::buffer_cast<char*>(*bfrPtr);
        delete data;
    }
    pool.clear();
    return -ret;
}

void TgtIntf::tgtReturnReadBuffer(const boost::shared_ptr<boost::asio::mutable_buffer> &b)
{
    _garbageCollector->release(b);
}

int TgtIntf::tgtRead(boost::shared_ptr<boost::asio::mutable_buffer> &b)
{
    int ret = 0;
    if (_incomingData.waitDequeue(b, 1) == true)
    {
        ret = boost::asio::buffer_size(*b);
        char* data = boost::asio::buffer_cast<char*>(*b);
        data[ret] = 0;
#ifdef CB_TRAP_TO_FILE
        _trapFile.write(data, ret);
#endif
    }
    return ret;
}

int TgtIntf::tgtWrite(const char* szWriteData, int nBytes)
{
    int ret = 0;
    if (nBytes > 0)
    {
        boost::shared_ptr<boost::asio::mutable_buffer> b;
        _bufferPool.dequeue(b);
        char* data = boost::asio::buffer_cast<char*>(*b);
        *b = boost::asio::buffer(data, TGT_BUFFER_SIZE - 1);

        boost::asio::buffer_copy(*b, boost::asio::buffer(szWriteData, nBytes));
        *b = boost::asio::buffer(*b, nBytes);
        _outgoingData.enqueue(b);
    }
    return ret;
}

void TgtIntf::tgtAttemptReconnect()
{
    bool reconnected = false;
    std::string title;
    std::string newTitle;
    tgtGetTitle(&title);
    newTitle = title;
    newTitle.append(" Disconnected");
    emit updateTitleSignal(newTitle.c_str());
    while ((reconnected == false) && (_running == true))
    {
        try
        {
            tgtMakeConnection();
            reconnected = true;
        }
        catch (const std::exception &e)
        {
            // update status bar
            emit updateStatusSignal(e.what());
        }
        if ((reconnected == false) && (_running == true))
        {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
        }
    }
    emit updateTitleSignal(title.c_str());
}

int TgtIntf::tgtDisconnect(bool running)
{
    _running = running;
    boost::mutex::scoped_lock guard(_disconnectMutex);
    return tgtBreakConnection();
}

