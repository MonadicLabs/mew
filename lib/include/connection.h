#pragma once

#include "identifiable.h"
#include "port.h"

namespace mew
{
    class Connection : public EntityIdentifiable
    {
    public:
        Connection( Port* a, Port* b )
            :_a(a), _b(b)
        {
            _a->connectTo( _b );
            _b->connectFrom( _a );
        }

        virtual ~Connection()
        {
            _a->disconnectTo( _b );
            _b->disconnectFrom( _a );
        }

    private:
        Port * _a;
        Port * _b;

    protected:

    };
}
