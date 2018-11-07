#pragma once

#include <abstractqueue.h>
// #include <workstealingqueue_impl2.h>
#include <concurrentqueue.h>
#include <deque>
#include <mutex>

namespace mew
{
    template< class T >
    class WorkStealingQueue : public AbstractQueue<T>
    {
    public:
        WorkStealingQueue()
        {

        }

        virtual ~WorkStealingQueue()
        {

        }

        bool push( const T& item )
        {
            // return _impl.push( item );
//            return _impl.try_enqueue( item );
            std::unique_lock<std::mutex> lock(_implMtx);
            _impl.push_back( item );
            return true;
        }

        bool pop( T& item )
        {
//            return _impl.pop( item );
//            return _impl.try_dequeue( item );
            std::unique_lock<std::mutex> lock(_implMtx);
            if( _impl.size() > 0 )
            {
                item = _impl.front();
                _impl.pop_front();
                return true;
            }
            return false;
        }

        bool steal( T& item )
        {
//            return _impl.pop( item );
//            return _impl.try_dequeue( item );
//            return false;
            std::unique_lock<std::mutex> lock(_implMtx);
            if( _impl.size() > 0 )
            {
                item = _impl.back();
                _impl.pop_back();
                return true;
            }
            return false;
        }

        size_t size()
        {
//            return _impl.size();
//            return _impl.size_approx();
//            return 0;
            std::unique_lock<std::mutex> lock(_implMtx);
            return _impl.size();
        }

    private:
        // IMPL 2
        // tp::MPMCBoundedQueue<T> _impl;
        // moodycamel::ConcurrentQueue<T> _impl;
        std::deque< T > _impl;
        std::mutex _implMtx;

    protected:

    };
}
