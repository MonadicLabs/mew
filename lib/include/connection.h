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
            _topic = _b->connectReceivingEnd( _a );
            _a->connectEmittingEnd( _b );
        }

        virtual ~Connection()
        {
            _a->disconnectEmittingEnd( _b );
            _b->disconnectReceivingEnd( _a );
        }

    private:
        Port * _a;
        Port * _b;
        std::string _topic;

    protected:

    };
}
