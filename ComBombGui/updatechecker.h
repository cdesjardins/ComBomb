/*
    ComBomb - Terminal emulator
    Copyright (C) 2015  Chris Desjardins
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
#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H
#include <qobject.h>
#include <memory>
#include <thread>
#include <boost/asio.hpp>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    ~UpdateChecker();

    static void checkForNewVersion();
    static UpdateChecker* get()
    {
        return _inst.get();
    }

signals:
    void newVersionAvailable();

private:
    UpdateChecker();
    UpdateChecker(const UpdateChecker&) = delete;
    void checkForNewVersionThread();
    bool processResponseHeader(boost::asio::ip::tcp::socket& socket, boost::asio::streambuf& response);
    void replyFinished(const std::string& comBombVersion);

    static std::unique_ptr<UpdateChecker> _inst;
    std::unique_ptr<std::thread> _checkThread;
    bool _running;
};

#endif // UPDATECHECKER_H
