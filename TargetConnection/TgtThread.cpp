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
#include <thread>
#include <sstream>

TgtThread::TgtThread()
    : _threadRun(true)
{
}

TgtThread::~TgtThread()
{
    join();
}

void TgtThread::join()
{
    //qDebug(toString("join", _thread.get()).c_str());
    _threadRun = false;
    if ((_thread != NULL) && (_thread->joinable() == true) && (std::this_thread::get_id() != _thread->get_id()))
    {
        //qDebug(toString("join for reals", _thread.get()).c_str());
        _thread->join();
    }
}

void TgtThread::start()
{
    //qDebug(toString("start").c_str());
}

void TgtThread::finalize()
{
    //qDebug(toString("finalize").c_str());
}

std::string TgtThread::toString(const std::string& tag, std::thread* thr)
{
    std::stringstream oss;
    oss << tag << ": ";
    if (thr == NULL)
    {
        oss << std::this_thread::get_id();
    }
    else
    {
        oss << thr->get_id();
    }
    oss << " " << _threadRun;
    return oss.str();
}

