#include "QTerminal/TgtIntf.h"
#include <boost/bind.hpp>
/*
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/
#define TGT_BUFFER_SIZE   4096
TgtIntf::TgtIntf(void)
    :_running(true)
{
    for (size_t i = 0; i < 4096; i++)
    {
        char* buffer = new char[TGT_BUFFER_SIZE];
        _bufferPool.enqueue(boost::asio::mutable_buffer(buffer, TGT_BUFFER_SIZE - 1));
    }

    _bufferPool.dequeue(_currentIncomingBuffer);

    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
    _running = false;
    _bufferPool.iterate(boost::bind(&TgtIntf::deleteBuffersFunctor, _1));
}

int TgtIntf::deleteBuffersFunctor(std::list<boost::asio::mutable_buffer> &pool)
{
    int ret = pool.size();
    std::list<boost::asio::mutable_buffer>::iterator it;
    for (it = pool.begin(); it != pool.end(); it++)
    {
        char* data = boost::asio::buffer_cast<char*>(*it);
        delete data;
    }
    pool.clear();
    return -ret;
}

void TgtIntf::TgtReturnReadBuffer(const boost::asio::mutable_buffer &b)
{
    _bufferPool.enqueue(boost::asio::buffer(b, TGT_BUFFER_SIZE - 1));
}

int TgtIntf::TgtRead(boost::asio::mutable_buffer &b)
{
    int ret = 0;
    if (_incomingData.waitDequeue(b, 1) == true)
    {
        ret = boost::asio::buffer_size(b);
        char* data = boost::asio::buffer_cast<char*>(b);
        data[ret] = 0;
    }
    return ret;
}

int TgtIntf::TgtWrite(const char* szWriteData, int nBytes)
{
    int ret = 0;
    if (nBytes > 0)
    {
        boost::asio::mutable_buffer b;
        _bufferPool.dequeue(b);
        boost::asio::buffer_copy(b, boost::asio::buffer(szWriteData, nBytes));
        _outgoingData.enqueue(boost::asio::buffer(b, nBytes));
    }
    return ret;
}

void TgtIntf::TgtAttemptReconnect()
{
    bool reconnected = false;
    while ((reconnected == false) && (_running == true))
    {
        try
        {
            TgtMakeConnection();
            reconnected = true;
        }
        catch (const std::exception &e)
        {
            // update status bar
            //qDebug(e.what());
        }
        if ((reconnected == false) && (_running == true))
        {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
        }
    }
}
