#pragma once

#include <deque>
#include <iostream>
using namespace std;

#include "abstractqueue.h"

namespace mew
{
    template< class T >
    class FIFOQueue : public AbstractQueue<T>
    {
    public:
        FIFOQueue()
        {

        }

        virtual ~FIFOQueue()
        {

        }

        bool push( const T& item )
        {
//            return _impl.Push( item );
//            cerr << "fifo push" << endl;
            _impl.push_back( item );
            return true;
        }

        bool pop( T& item )
        {
            bool ret = false;
//            cerr << "fifo pop" << endl;
            if( _impl.size() > 0 )
            {
                item = _impl.front();
                _impl.pop_front();
                ret = true;
            }
            return ret;
        }

        bool steal( T& item )
        {
//            return _impl.Steal( item );
            return false;
        }

        size_t size()
        {
//            return _impl.size();
            return _impl.size();
        }

    private:
        std::deque<T> _impl;

    protected:

    };
}
