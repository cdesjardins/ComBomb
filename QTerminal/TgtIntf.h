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
#ifndef TGTINTF_H
#define TGTINTF_H

#define CB_DEFAULT_TERM_WIDTH 80
#define CB_DEFAULT_TERM_HEIGHT 25

#include <boost/asio/buffer.hpp>
#include "threadsafequeue.h"
#include "QueuePtr/RefCntBufferPool.h"
#include "unparam.h"
#include <QWidget>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>

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

    TgtIntf(const std::shared_ptr<const TgtConnectionConfigBase>& config);
    virtual ~TgtIntf(void);

    virtual int tgtRead(boost::intrusive_ptr<RefCntBuffer>& b);
    virtual int tgtWrite(const char* szWriteData, int nBytes);
    virtual void tgtGetTitle(std::string* szTitle) = 0;
    // Override to handle window resize events
    virtual void tgtWindowResize(int cols, int rows)
    {
        UNREF_PARAM(cols);
        UNREF_PARAM(rows);
    }

    virtual int tgtGetBytesRx()
    {
        return m_nTotalRx;
    }

    virtual int tgtGetBytesTx()
    {
        return m_nTotalTx;
    }

    std::shared_ptr<const TgtConnectionConfigBase> getConfig()
    {
        return _connectionConfig;
    }

    void tgtDisconnect();

signals:
    void updateStatusSignal(QString);
    void updateTitleSignal(QString);
protected:
    void tgtAttemptReconnect();
    void connectionManagerThread();
    bool connectionManagerWait(std::chrono::system_clock::time_point startTime);
    void connectionManagerStop();
    void updateTitle(bool disconnected);
    virtual void tgtMakeConnection() = 0;
    virtual void tgtBreakConnection() = 0;

    int m_nTotalTx;
    int m_nTotalRx;
    ThreadSafeQueue<boost::intrusive_ptr<RefCntBuffer> > _incomingData;
    ThreadSafeQueue<boost::intrusive_ptr<RefCntBuffer> > _outgoingData;
    std::shared_ptr<RefCntBufferPool> _bufferPool;
    std::shared_ptr<const TgtConnectionConfigBase> _connectionConfig;
    std::unique_ptr<std::thread> _connectionManagerThread;
    volatile bool _connectionManagerThreadRun;
    std::mutex _connectionManagerMutex;
    volatile bool _attemptReconnect;
};

#endif // TGTINTF_H
