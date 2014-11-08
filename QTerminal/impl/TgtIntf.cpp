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
    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
    _running = false;
}

int TgtIntf::tgtRead(boost::intrusive_ptr<RefCntBuffer> &b)
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
                int sentBytes = boost::asio::buffer_copy(b->_buffer, boost::asio::buffer(szWriteData + totalSentBytes, nBytes - totalSentBytes));
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

