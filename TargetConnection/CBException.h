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
#ifndef CBEXCEPTION_H
#define CBEXCEPTION_H

#include <boost/asio/buffer.hpp>
#include <boost/format.hpp>

#define CB_EXCEPTION(type)                       type(__FILE__, __LINE__)
#define CB_EXCEPTION_INT(type, data)             type(__FILE__, __LINE__, data)
#define CB_EXCEPTION_STR(type, str)              type(__FILE__, __LINE__, str)

class CBException : public std::exception
{
public:
    static CBException CbExcp(const char* file, const int line, const char* str)
    {
        return CBException(file, line, str);
    }

    virtual const char* what() const throw()
    {
        return _what;
    }

protected:
    CBException(const char* file, const int line)
    {
        init(file, line);
    }

    CBException(const char* file, const int line, const char* str)
    {
        init(file, line);
        copyStr(" ");
        copyStr(str);
    }

    CBException(const char* file, const int line, const int data)
    {
        init(file, line);
        copyInt(data);
    }

    void copyStr(const char* str)
    {
        int len = strlen(str);
        int numBytesCopied = boost::asio::buffer_copy(_whatBuffer, boost::asio::buffer(str, len));
        _whatBuffer = boost::asio::buffer(_whatBuffer + numBytesCopied);
    }

    void copyInt(const int data)
    {
        boost::format f(": %i");
        copyStr(str(f % data).c_str());
    }

    void init(const char* file, const int line)
    {
        memset(_what, 0, sizeof(_what));
        _whatBuffer = boost::asio::buffer(_what, sizeof(_what) - 1);
#ifdef CB_LOG_EXECP_FNL
        copyStr(file);
        copyInt(line);
#else
        UNREF_PARAM(file);
        UNREF_PARAM(line);
#endif
    }

    boost::asio::mutable_buffer _whatBuffer;
    char _what[2048];
};

#endif // CBEXCEPTION_H
