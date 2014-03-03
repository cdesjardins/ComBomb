#ifndef GARBAGECOLLECTOR_H
#define GARBAGECOLLECTOR_H

#ifndef Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#endif
#include "ThreadSafeQueue.h"

template <class T> class GarbageCollector
{
public:
    static boost::shared_ptr<GarbageCollector> createGargabageCollector(ThreadSafeQueue<boost::shared_ptr<T> > *queue)
    {
        boost::shared_ptr<GarbageCollector> ret(new GarbageCollector(queue));
        ret->_thread.reset(new boost::thread(boost::bind(&GarbageCollector::collectionThread, ret.get())));
        return ret;
    }
    void release(const boost::shared_ptr<T> &data)
    {
        _holdingQueue.enqueue(data);
    }

    ~GarbageCollector()
    {
        _threadRunning = false;
        _thread->join();
    }

protected:
    void collectionThread()
    {
        while (_threadRunning)
        {
            boost::shared_ptr<T> data;
            if (_holdingQueue.waitDequeue(data, 10) == true)
            {
                if (data.use_count() == 1)
                {
                    _queue->enqueue(data);
                }
                else
                {
                    _holdingQueue.enqueue(data);
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                }
            }
        }
    }

    GarbageCollector(ThreadSafeQueue<boost::shared_ptr<T> > *queue)
        : _queue(queue),
          _threadRunning(true)
    {

    }

    ThreadSafeQueue<boost::shared_ptr<T> > *_queue;
    ThreadSafeQueue<boost::shared_ptr<T> > _holdingQueue;
    boost::scoped_ptr<boost::thread> _thread;
    bool _threadRunning;
};

#endif // GARBAGECOLLECTOR_H
