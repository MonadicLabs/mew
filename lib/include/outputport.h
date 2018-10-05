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

    virtual void connectTo( Port* p )
    {
        cerr << "connect to " << p << endl;
        _pub = p->getPubAddress();
        cerr << "got pub addr=" << _pub << endl;
    }

    virtual void disconnectTo( Port* p )
    {
        cerr << "disconnect to " << p << endl;
    }

    virtual bool write(cpp17::any v)
    {
        if( _pub.size() )
        {
            WorkSpace * ws = _parent->context();
            cerr << "_pub=" << _pub << endl;
            ws->getRuntime()->publish( _pub, v );
        }
    }

private:
    std::string _pub;

};
}
