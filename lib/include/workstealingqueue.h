#pragma once

#include <workstealingqueue_impl2.h>

namespace mew
{
    template< class T >
    class WorkStealingQueue
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
            return _impl.Push( item );
        }

        bool pop( T& item )
        {
            return _impl.Pop( item );
        }

        bool steal( T& item )
        {
            return _impl.Steal( item );
        }

        size_t size()
        {
            return _impl.size();
        }

    private:
        // IMPL 2
        WorkStealingStack<T> _impl;

    protected:

    };
}
