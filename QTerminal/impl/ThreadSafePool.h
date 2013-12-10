#ifndef THREADSAFEPOOL_H
#define THREADSAFEPOOL_H

#include "ThreadSafeQueue.h"
#include "garbagecollector.h"

template <class T> class ThreadSafePool : public ThreadSafeQueue<boost::shared_ptr<T> >
{
public:

protected:
    ThreadSafePool()
    {
        _garbageCollector = GarbageCollector<T>::createGargabageCollector(this);
    }
    void addToPool(int num)
    {
        for (int i = 0; i < num; i++)
        {
            enqueue(allocateBuffer());
        }
    }

    virtual bool dequeue(boost::shared_ptr<boost::asio::mutable_buffer> &data)
    {
        bool ret = ThreadSafeQueue<boost::shared_ptr<T> >::dequeue(data);
        boost::shared_ptr<boost::asio::mutable_buffer> g = data;
        _garbageCollector->release(g);
        return ret;
    }

    virtual boost::shared_ptr<T> allocateBuffer() const = 0;

    boost::shared_ptr<GarbageCollector<T> > _garbageCollector;
};

#endif // THREADSAFEPOOL_H
