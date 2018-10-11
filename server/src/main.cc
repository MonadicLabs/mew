#include <iostream>
using namespace std;

#include "mewserver.h"
#include "wsserver.h"
#include "graph.h"

using namespace mew;


int main( int argc, char** argv )
{

    mew::Server server;
    server.createNewWorkspace();

    //
    // TEST
    //
    mew::Graph * g = server.workspace()->createEmptyGraph();
    g->addNode( Node::create("Clock") );

    WebSocketServer wss(&server);
    // wss.start();

	return 0;
}
