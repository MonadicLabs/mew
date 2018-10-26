
#include "udpsink.h"
#include "workspace.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>

mew::UDPSink::UDPSink(WorkSpace *ctx)
    :Node(ctx)
{
    declare_input( "in" );
    declare_parameter( "port", Value::NUMBER, 9940 );
    declare_parameter( "host", Value::STRING, "127.0.0.1" );
    _fd = -1;
}

mew::UDPSink::~UDPSink()
{

}

void mew::UDPSink::onContextChange(mew::WorkSpace *ctx)
{

}

void mew::UDPSink::onSetParameter(const string &paramName, Value value)
{
    cerr << "[UDPSink] onSetParameter ! paramName=" << paramName << " value=" << value.str() << endl;
    if( paramName == "port" || paramName == "host" )
    {
        init_udp_socket();
    }
}

void mew::UDPSink::onInput(const string &portName, any &v)
{
//    cerr << "USPSink onInput" << endl;
    Value content;
    try{
        content = cpp::any_cast<Value>( v );
        if( content.is(Value::BINARY) )
        {
#ifdef MEW_USE_PROFILING
        rmt_BeginCPUSampleDynamic( "UDP_SEND", 0);
#endif
            /* send the message to the server */
            int serverlen = sizeof(serveraddr);
            int n = sendto(_fd, &content.binary()[0], content.binary().size(), 0, (const sockaddr*)(&serveraddr), serverlen);
            if (n < 0)
              cerr << "ERROR in sendto" << endl;
#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
        }
    }
    catch ( cpp::bad_any_cast& e )
    {

    }
}

void mew::UDPSink::init_udp_socket()
{
    if( _fd >= 0 )
    {
        // Close previous socket first
        close( _fd );
        _fd = -1;
    }

    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);

    if ((_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        //diep("socket");
        cerr << ":( socket." << endl;

    struct hostent *server;

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname( getParameter("host").string().c_str() );
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", getParameter("host").string().c_str() );
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons( getParameter("port").number() );

}

REGISTER_NODE(UDPSink)
