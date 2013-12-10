#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#ifndef Q_MOC_RUN
#include <boost/asio/buffer.hpp>
#include <boost/smart_ptr.hpp>
#endif

#include "ThreadSafePool.h"


class BufferPool : public ThreadSafePool<boost::asio::mutable_buffer>
{
public:
    static boost::shared_ptr<BufferPool> createPool(int initialBufs)
    {
        boost::shared_ptr<BufferPool> ret(new BufferPool());
        ret->addToPool(initialBufs);
        return ret;
    }

    virtual bool dequeue(boost::shared_ptr<boost::asio::mutable_buffer> &data);

protected:
    BufferPool();
    virtual boost::shared_ptr<boost::asio::mutable_buffer> allocateBuffer() const;
};

#endif // BUFFERPOOL_H
