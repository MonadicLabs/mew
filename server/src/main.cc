#include <iostream>
using namespace std;

#include "mewserver.h"
#include "wsserver.h"
using namespace mew;

int main( int argc, char** argv )
{
//    mew::Server server();
       WebSocketServer wss(0);
    wss.start();

	return 0;
}
