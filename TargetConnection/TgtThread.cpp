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
#include "CDLogger/Logger.h"
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
    _threadRun = false;
    if ((_thread != nullptr) && (_thread->joinable() == true) && (std::this_thread::get_id() != _thread->get_id()))
    {
        _thread->join();
    }
}

void TgtThread::start()
{
    //cdLog(LogLevel::Debug) << toString("start");
}

void TgtThread::finalize()
{
    //cdLog(LogLevel::Debug) << toString("finalize");
}

std::string TgtThread::toString(const std::string& tag, std::thread* thr)
{
    std::stringstream oss;
    oss << tag << ": ";
    if (thr == nullptr)
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
