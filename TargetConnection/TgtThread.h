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

#ifndef TGTTHREAD_H
#define TGTTHREAD_H

#include <memory>
#include <string>
#include <thread>

class TgtThread
{
public:
    template <typename T>
    static std::shared_ptr<TgtThread> create(T function)
    {
        std::shared_ptr<TgtThread> ret(new TgtThread());
        ret->_thread.reset(new std::thread(std::bind(&TgtThread::theThread<T>, ret.get(), function)));
        return ret;
    }

    ~TgtThread();
    void join();

    bool threadRun()
    {
        return _threadRun;
    }

    std::string toString(const std::string& tag, std::thread* thr = nullptr);
    void start();
    void finalize();
protected:
    TgtThread();
    template <typename T>
    void theThread(T function)
    {
        bool cont;
        start();
        do
        {
            cont = function();
        } while ((_threadRun == true) && (cont == true));
        finalize();
    }

    std::unique_ptr<std::thread> _thread;
    volatile bool _threadRun;

private:
    TgtThread& operator=(TgtThread const&);
    TgtThread(TgtThread const&);
};
#endif // TGTTHREAD_H
