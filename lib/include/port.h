#pragma once

#include <iostream>
using namespace std;

namespace mew
{
class Node;
class Port
{
public:
    Port( Node* parent = 0 )
        :_parent(parent)
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

private:

protected:
    Node* _parent;

};

class InputPort : public Port
{
public:
    InputPort( Node* parent, void* subCtx )
        :Port(parent), _subCtx(subCtx)
    {

    }

    virtual ~InputPort()
    {

    }

    virtual void connectFrom( Port* p )
    {
        cerr << "connect from " << p << endl;
    }

    virtual void disconnectFrom( Port* p )
    {
        cerr << "disconnect from " << p << endl;
    }

    void* _subCtx;

protected:

};

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
    }

    virtual void disconnectTo( Port* p )
    {
        cerr << "disconnect to " << p << endl;
    }

};

class ParameterPort : public InputPort
{
public:
    ParameterPort( Node* parent, void* subCtx )
        :InputPort(parent, subCtx)
    {

    }

    virtual ~ParameterPort()
    {

    }

private:

protected:

};

}
