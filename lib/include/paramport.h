#pragma once

#include "inputport.h"

namespace mew
{
class ParameterPort : public InputPort
{
public:
    ParameterPort( Node* parent, void* subCtx )
        :InputPort(parent, 1)
    {

    }

    virtual ~ParameterPort()
    {

    }

private:

protected:

};

}
