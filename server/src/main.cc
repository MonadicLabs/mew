#include <iostream>
using namespace std;

#include "mewserver.h"
#include "wsserver.h"
using namespace mew;

int main( int argc, char** argv )
{

    mew::Server server;
    server.createNewWorkspace();

    //
    // TEST
    //
    server.workspace()->createEmptyGraph();

    WebSocketServer wss(&server);
    wss.start();

	return 0;
}
