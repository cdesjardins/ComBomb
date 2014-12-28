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

#include "TgtThread.h"
#include <QDebug>
TgtThread::TgtThread()
    : _threadRun(true)
{
    qDebug("Thread start");
}

TgtThread::~TgtThread()
{
    join();
}

void TgtThread::join()
{
    qDebug("Thread join");
    _threadRun = false;
    if ((_thread != NULL) && (_thread->joinable() == true) && (boost::this_thread::get_id() != _thread->get_id()))
    {
        qDebug("Thread join for reals");
        _thread->join();
    }
}

void TgtThread::finalize()
{
    qDebug("Thread finalize");
}

void TgtThread::debug()
{
    std::stringstream oss;
    oss << "Thread debug: " << boost::this_thread::get_id() << " " << _threadRun;
    qDebug(oss.str().c_str());

}
