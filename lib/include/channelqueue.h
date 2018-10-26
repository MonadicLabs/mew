#pragma once

#include "abstractqueue.h"
#include <blockingconcurrentqueue.h>

namespace mew
{
    template< class T >
    class ChannelQueue : public AbstractQueue<T>
    {
    public:
        ChannelQueue()
        {

        }

        virtual ~ChannelQueue()
        {

        }

        bool push( const T& item )
        {
            return _impl.enqueue( item );
        }

        bool pop( T& item )
        {
            return _impl.try_dequeue( item );
//            return true;
        }

        bool steal( T& item)
        {
            return false;
        }

        size_t size()
        {
            return _impl.size_approx();
        }

    private:
        moodycamel::ConcurrentQueue<T> _impl;

    protected:

    };
}
