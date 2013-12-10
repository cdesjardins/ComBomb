#include "BufferPool.h"

#define TGT_BUFFER_SIZE   4096

BufferPool::BufferPool()
    : ThreadSafePool()
{
}

boost::shared_ptr<boost::asio::mutable_buffer> BufferPool::allocateBuffer() const
{
    char* buffer = new char[TGT_BUFFER_SIZE];
    boost::shared_ptr<boost::asio::mutable_buffer> bfrPtr(new boost::asio::mutable_buffer(buffer, TGT_BUFFER_SIZE - 1));
    return bfrPtr;
}

bool BufferPool::dequeue(boost::shared_ptr<boost::asio::mutable_buffer> &data)
{
    bool ret = ThreadSafePool<boost::asio::mutable_buffer>::dequeue(data);

    char* z = boost::asio::buffer_cast<char*>(*data);
    *data = boost::asio::buffer(z, TGT_BUFFER_SIZE - 1);

    return ret;
}
