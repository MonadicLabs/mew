#pragma once

#include <vector>

namespace mew
{
    class IOPoller
    {
    public:
        IOPoller()
        {

        }

        virtual ~IOPoller()
        {

        }

        virtual void add( int fd )
        {

        }

        virtual void remove( int fd )
        {

        }

        virtual std::vector< int > poll( double timeout_sec )
        {
            std::vector<int> ret;

            return ret;
        }

    private:

    protected:

    };
}
