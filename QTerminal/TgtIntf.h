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

#ifndef Q_MOC_RUN
#include <boost/asio/buffer.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#endif
#include "ThreadSafeQueue.h"
#include "RefCntBufferPool.h"
#include "../unparam.h"
#include <QWidget>
#include <fstream>

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

    TgtIntf(const boost::shared_ptr<const TgtConnectionConfigBase>& config);
    virtual ~TgtIntf(void);

    virtual int tgtRead(boost::intrusive_ptr<RefCntBuffer>& b);
    virtual int tgtWrite(const char* szWriteData, int nBytes);
    virtual void tgtGetTitle(std::string* szTitle) = 0;
    virtual int tgtGetBytesRx()
    {
        return m_nTotalRx;
    }

    virtual int tgtGetBytesTx()
    {
        return m_nTotalTx;
    }

    boost::shared_ptr<const TgtConnectionConfigBase> getConfig()
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
    bool connectionManagerWait();
    void connectionManagerStop();
    virtual void tgtMakeConnection() = 0;
    virtual void tgtBreakConnection() = 0;

    int m_nTotalTx;
    int m_nTotalRx;
    ThreadSafeQueue<boost::intrusive_ptr<RefCntBuffer> > _incomingData;
    ThreadSafeQueue<boost::intrusive_ptr<RefCntBuffer> > _outgoingData;
    boost::shared_ptr<RefCntBufferPool> _bufferPool;
    boost::shared_ptr<const TgtConnectionConfigBase> _connectionConfig;
    boost::scoped_ptr<boost::thread> _connectionManagerThread;
    volatile bool _connectionManagerThreadRun;
    boost::mutex _connectionManagerMutex;
    boost::condition_variable _connectionManagerCondition;
    volatile bool _connectionManagerSignal;
};

#endif // TGTINTF_H
