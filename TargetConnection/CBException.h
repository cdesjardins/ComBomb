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
        //copyStr(file);
        //copyInt(line);
    }
    boost::asio::mutable_buffer _whatBuffer;
    char _what[2048];
};

#endif // CBEXCEPTION_H
