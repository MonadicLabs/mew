#pragma once

#include <abstractqueue.h>
// #include <workstealingqueue_impl2.h>
#include "mpmc_bounded_queue.h"

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
            return _impl.push( item );
        }

        bool pop( T& item )
        {
            return _impl.pop( item );
        }

        bool steal( T& item )
        {
            return _impl.pop( item );
        }

        size_t size()
        {
//            return _impl.size();
            return 0;
        }

    private:
        // IMPL 2
        tp::MPMCBoundedQueue<T> _impl;

    protected:

    };
}
