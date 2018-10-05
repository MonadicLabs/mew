#pragma once

#include <iostream>
using namespace std;

#include <cppbackports/any.h>
#include <concurrentqueue.h>

#include "identifiable.h"

namespace mew
{
class Node;
class Port : public EntityIdentifiable
{

    friend class Node;

public:
    Port( Node* parent = 0 )
        :_parent(parent), _label("")
    {

    }

    virtual ~Port()
    {

    }

    virtual void connectTo( Port* p )
    {}

    virtual void connectFrom( Port* p )
    {}

    virtual void disconnectTo( Port* p )
    {}

    virtual void disconnectFrom( Port* p )
    {}

    virtual bool write( cpp17::any v )
    {
        return false;
    }

    virtual bool read( cpp17::any& v )
    {
        return false;
    }

    virtual std::string getPubAddress()
    {
        return "";
    }

private:

protected:
    Node* _parent;
    std::string _label;

};

}
