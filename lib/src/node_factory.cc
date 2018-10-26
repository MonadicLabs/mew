
#include "node.h"

#include "udpsrc/udpsrc.h"
#include "udpsink/udpsink.h"
#include "clock/clock.h"
#include "console/console.h"

mew::Node *mew::Node::create(const string &nodeType)
{
    Node * ret = nullptr;
    cerr << "Attempting to create node of type: " << nodeType << endl;
    if( nodeType == "UDPSrc" )
    {
        ret = new UDPSrc();
    }
    else if( nodeType == "UDPSink" )
    {
        ret = new UDPSink();
    }
    else if( nodeType == "Clock" )
    {
        ret = new Clock();
    }
    else if( nodeType == "Console" )
    {
        ret = new Console();
    }
    return ret;
}
