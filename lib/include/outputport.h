#pragma once

#include <iostream>
using namespace std;

#include "port.h"
#include "workspace.h"

namespace mew
{
class OutputPort : public Port
{
public:
    OutputPort( Node* parent )
        :Port(parent)
    {

    }

    virtual ~OutputPort()
    {

    }

    virtual void connectEmittingEnd( Port* otherEnd )
    {
        cerr << "connect to " << otherEnd << endl;
        _pub = otherEnd->pub_address();
        cerr << "got pub addr=" << _pub << endl;
    }

    virtual bool write(cpp17::any v)
    {
        if( _pub.size() )
        {
            WorkSpace * ws = _parent->context();
            // cerr << "_pub=" << _pub << endl;
            ws->getRuntime()->channel_write( _pub, v );
        }
    }

private:
    std::string _pub;

};
}
