#include "QTerminal/TgtIntf.h"
#include <boost/bind.hpp>
/*
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/

#define CB_TGT_INTF_NUM_BUFFS  1024
#define CB_TGT_INTF_BUFF_SIZE  4096
TgtIntf::TgtIntf(const boost::shared_ptr<const TgtConnectionConfigBase> &config)
    : _bufferPool(new RefCntBufferPool(CB_TGT_INTF_NUM_BUFFS, CB_TGT_INTF_BUFF_SIZE)),
    _connectionConfig(config),
    _running(true)
{
#ifdef CB_TRAP_TO_FILE
    std::string trapFileName("debug.cbd");
    _trapFile.open(trapFileName.c_str(), std::ios::out | std::ios::binary);
#endif

    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
    _running = false;
#ifdef CB_TRAP_TO_FILE
    _trapFile.close();
#endif
}

int TgtIntf::tgtRead(boost::intrusive_ptr<RefCntBuffer> &b)
{
    int ret = 0;
    if (_incomingData.dequeue(b, 1) == true)
    {
        ret = boost::asio::buffer_size(b->_buffer);
        char* data = boost::asio::buffer_cast<char*>(b->_buffer);
        data[ret] = 0;
#ifdef CB_TRAP_TO_FILE
        _trapFile.write(data, ret);
#endif
        m_nTotalRx += ret;
    }
    return ret;
}

int TgtIntf::tgtWrite(const char* szWriteData, int nBytes)
{
    int ret = 0;
    if (nBytes > 0)
    {
        int totalSentBytes = 0;
        do
        {
            boost::intrusive_ptr<RefCntBuffer> b;
            if (_bufferPool->dequeue(b, 1) == true)
            {
                int sentBytes = boost::asio::buffer_copy(b->_buffer, boost::asio::buffer(szWriteData + totalSentBytes, nBytes - totalSentBytes));
                b->_buffer = boost::asio::buffer(b->_buffer, sentBytes);
                _outgoingData.enqueue(b);
                m_nTotalTx += sentBytes;
                totalSentBytes += sentBytes;
            }
        } while (totalSentBytes < nBytes);
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

