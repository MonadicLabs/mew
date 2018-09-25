#pragma once

namespace mew
{
    template< class T >
    class AbstractQueue
    {
    public:
        AbstractQueue()
        {

        }

        virtual ~AbstractQueue()
        {

        }

        virtual bool push( const T& item )
        {

        }

        virtual bool pop( T& item )
        {

        }

        virtual bool steal( T& item )
        {

        }

        virtual size_t size()
        {

        }

    private:

    protected:

    };
}
