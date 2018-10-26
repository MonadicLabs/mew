
#include "udpsrc.h"
#include "workspace.h"

#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

mew::UDPSrc::UDPSrc(WorkSpace *ctx)
    :Node(ctx)
{
    declare_output( "out" );
    declare_parameter( "port", Value::NUMBER, 9940 );
    _fd = -1;

    // LOOPBACK TEST
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);

    if ((_lo=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        //diep("socket");
        cerr << ":( socket." << endl;

    struct hostent *server;

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname( "127.0.0.1" );
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", getParameter("host").string().c_str() );
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons( 5000 );
    //
}

mew::UDPSrc::~UDPSrc()
{

}

void mew::UDPSrc::onContextChange(mew::WorkSpace *ctx)
{
    mew::Mew * m = ctx->getRuntime();
    std::function<void(mew::Mew*,int)> f = [this](mew::Mew*, int fd){
        char buf[4096];
        // while(true)
        {
            memset( buf, 0, 4096 );
            int cc = read( fd, buf, 4096 );
            //cerr << "cc=" << cc << endl;
            if( cc <= 0 )
            {
                // break;
            }
            else
            {
                // cerr << "cc=" << cc << " buf=" << buf << endl;
                Value ret(cc);
                ret.setBinary( buf, cc );
                this->out("out")->write( ret );

                /*
                // LOOPBACK TEST
                cerr << "cc=" << cc << endl;
                int serverlen = sizeof(serveraddr);
                int n = sendto(_lo, buf, cc, 0, (const sockaddr*)(&serveraddr), serverlen);
                if (n < 0)
                  cerr << "ERROR in sendto" << endl;
                //
                */

            }
        }
    };
    void * popo = m->io( f, _fd );
}

void mew::UDPSrc::onSetParameter(const string &paramName, Value value)
{
    cerr << "[UDPSrc] onSetParameter ! paramName=" << paramName << " value=" << value.str() << endl;
    if( paramName == "port" )
    {
        init_udp_socket();
    }
}

void mew::UDPSrc::init_udp_socket()
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

    setNonBlocking( _fd );

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    uint16_t bindPort = (uint16_t)(getParameter("port").number());
    cerr << "bindPort=" << bindPort << endl;
    si_me.sin_port = htons( bindPort );
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    cerr << "Binding on port " << ntohs(si_me.sin_port) << endl;

    if (::bind(_fd, (const sockaddr*)(&si_me), sizeof(si_me))==-1)
        cerr << ":( bind" << endl;

}

int mew::UDPSrc::setNonBlocking(int fd)
{
    int flags;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}



REGISTER_NODE(UDPSrc)
