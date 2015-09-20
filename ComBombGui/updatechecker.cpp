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
#include "updatechecker.h"
#include "versioning.h"
#include "CDLogger/Logger.h"

#define VERSION_HOST "blog.chrisd.info"
#define VERSION_PATH "/download/combombversion/latest.txt"
std::unique_ptr<UpdateChecker> UpdateChecker::_inst;

UpdateChecker::UpdateChecker()
    : _running(true)
{
}

UpdateChecker::~UpdateChecker()
{
    _running = false;
    if ((_checkThread != nullptr) && (_checkThread->joinable() == true))
    {
        _checkThread->join();
    }
}

void UpdateChecker::checkForNewVersion()
{
    if (_inst == nullptr)
    {
        _inst.reset(new UpdateChecker());
        _inst->_checkThread.reset(new std::thread(&UpdateChecker::checkForNewVersionThread, _inst.get()));
    }
}

bool UpdateChecker::processResponseHeader(boost::asio::ip::tcp::socket& socket, boost::asio::streambuf& response)
{
    bool ret = true;
    std::istream responseStream(&response);
    std::string httpVersion;
    responseStream >> httpVersion;
    unsigned int statusCode;
    responseStream >> statusCode;
    std::string statusMessage;
    std::getline(responseStream, statusMessage);
    if ((!responseStream) || (httpVersion.substr(0, 5) != "HTTP/") || (statusCode != 200))
    {
        cdLog(LogLevel::Info) << "Invalid response";
        ret = false;
    }
    else
    {
        boost::asio::read_until(socket, response, "\r\n\r\n");
        std::string header;
        while ((_running == true) && std::getline(responseStream, header) && (header != "\r"))
        {
        }
    }
    return ret;
}

void UpdateChecker::checkForNewVersionThread()
{
    try
    {
        boost::asio::io_service ioService;

        boost::asio::ip::tcp::resolver resolver(ioService);
        boost::asio::ip::tcp::resolver::query query(VERSION_HOST, "http");
        boost::asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);

        boost::asio::ip::tcp::socket socket(ioService);
        boost::asio::connect(socket, endpointIterator);

        boost::asio::streambuf request;
        std::ostream requestStream(&request);
        requestStream << "GET " << VERSION_PATH << " HTTP/1.0\r\n";
        requestStream << "Host: " << VERSION_HOST << "\r\n";
        requestStream << "Accept: */*\r\n";
        requestStream << "Connection: close\r\n\r\n";
        boost::asio::write(socket, request);

        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        if (processResponseHeader(socket, response) == true)
        {
            std::string comBombVersion;
            if (response.size() > 0)
            {
                std::istream responseStream(&response);
                responseStream >> comBombVersion;
            }

            // Read until EOF, writing data to output as we go.
            boost::system::error_code error;
            while ((_running == true) && boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
            {
                std::string respStr;
                std::istream responseStream(&response);
                responseStream >> respStr;
                comBombVersion.append(respStr);
            }
            if (error == boost::asio::error::eof)
            {
                replyFinished(comBombVersion);
            }
            socket.close();
        }
    }
    catch (std::exception& e)
    {
        cdLog(LogLevel::Error) << "Check for new version exception: " << e.what();
    }
}

void UpdateChecker::replyFinished(const std::string& comBombVersion)
{
    int32_t currentVersion = parseVersionStr(getVersion());
    int32_t latestVersion = parseVersionStr(comBombVersion);
    if ((latestVersion != -1) && (currentVersion != -1) && (latestVersion > currentVersion))
    {
        emit newVersionAvailable();
    }
}

