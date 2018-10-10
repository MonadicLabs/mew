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

    virtual void connectEmittingEnd( Port* otherEnd )
    {

    }

    virtual std::string connectReceivingEnd( Port* otherEnd )
    {
        return "";
    }

    virtual void disconnectEmittingEnd( Port* otherEnd )
    {

    }

    virtual void disconnectReceivingEnd( Port* otherEnd )
    {

    }

    virtual bool write( cpp17::any v )
    {
        return false;
    }

    virtual bool read( cpp17::any& v )
    {
        return false;
    }

    virtual std::string pub_address()
    {
        return "";
    }

private:

protected:
    Node* _parent;
    std::string _label;

};

}
