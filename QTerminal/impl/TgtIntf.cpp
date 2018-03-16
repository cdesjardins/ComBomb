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
#include "../TgtIntf.h"

/*
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/

#define CB_TGT_INTF_NUM_BUFFS  1024
#define CB_TGT_INTF_BUFF_SIZE  4096
TgtIntf::TgtIntf(const std::shared_ptr<const TgtConnectionConfigBase>& config)
    : _bufferPool(new RefCntBufferPool(CB_TGT_INTF_NUM_BUFFS, CB_TGT_INTF_BUFF_SIZE)),
    _connectionConfig(config),
    _connectionManagerThreadRun(true),
    _connectionManagerSignal(false)
{
    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
    connectionManagerStop();
}

int TgtIntf::tgtRead(boost::intrusive_ptr<RefCntBuffer>& b)
{
    int ret = 0;
    if (_incomingData.dequeue(b, 1) == true)
    {
        ret = boost::asio::buffer_size(b->_buffer);
        char* data = boost::asio::buffer_cast<char*>(b->_buffer);
        data[ret] = 0;
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
            if (_bufferPool->dequeue(b, 5) == true)
            {
                int sentBytes =
                    boost::asio::buffer_copy(b->_buffer,
                                             boost::asio::buffer(szWriteData + totalSentBytes,
                                                                 nBytes - totalSentBytes));
                b->_buffer = boost::asio::buffer(b->_buffer, sentBytes);
                _outgoingData.enqueue(b);
                m_nTotalTx += sentBytes;
                totalSentBytes += sentBytes;
            }
            else
            {
                break;
            }
        } while (totalSentBytes < nBytes);
    }
    return ret;
}

void TgtIntf::tgtAttemptReconnect()
{
    if (_connectionManagerThread == nullptr)
    {
        _connectionManagerThread.reset(new std::thread(std::bind(&TgtIntf::connectionManagerThread, this)));
    }

    std::unique_lock<std::mutex> lock(_connectionManagerMutex);
    _connectionManagerSignal = true;
    _connectionManagerCondition.notify_all();
}

void TgtIntf::tgtDisconnect()
{
    tgtBreakConnection();
    connectionManagerStop();
}

void TgtIntf::connectionManagerStop()
{
    _connectionManagerThreadRun = false;
    if ((_connectionManagerThread != nullptr) && (_connectionManagerThread->joinable() == true))
    {
        _connectionManagerThread->join();
    }
}

bool TgtIntf::connectionManagerWait()
{
    std::unique_lock<std::mutex> lock(_connectionManagerMutex);
    _connectionManagerCondition.wait_for(lock, std::chrono::milliseconds(1000));
    bool ret = _connectionManagerSignal;
    _connectionManagerSignal = false;
    return ret;
}

void TgtIntf::updateTitle(bool disconnected)
{
    std::string title;
    tgtGetTitle(&title);
    if (disconnected == true)
    {
        title.append(" Disconnected");
    }
    emit updateTitleSignal(title.c_str());
}

void TgtIntf::connectionManagerThread()
{
    while (_connectionManagerThreadRun)
    {
        if (connectionManagerWait() == true)
        {
            bool reconnected = false;
            while ((reconnected == false) && (_connectionManagerThreadRun == true))
            {
                updateTitle(true);
                try
                {
                    tgtBreakConnection();
                    tgtMakeConnection();
                    reconnected = true;
                }
                catch (const std::exception& e)
                {
                    // update status bar
                    emit updateStatusSignal(e.what());
                }
                if (reconnected == false)
                {
                    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
                    do
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    } while (std::chrono::system_clock::now() < (startTime + std::chrono::seconds(1)) &&
                             (_connectionManagerThreadRun == true));
                }
            }
            updateTitle(false);
        }
    }
}
