
#include "udpsrc.h"
#include "workspace.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

mew::UDPSrc::UDPSrc(WorkSpace *ctx)
    :Node(ctx)
{
    declare_output( "out" );
    declare_parameter( "port", Value::NUMBER, 9940 );
    _fd = -1;
}

mew::UDPSrc::~UDPSrc()
{

}

void mew::UDPSrc::onContextChange(mew::WorkSpace *ctx)
{
    mew::Mew * m = ctx->getRuntime();
    std::function<void(mew::Mew*,int)> f = [this](mew::Mew*, int fd){
        char buf[1024];
        // cerr << "fd=" << fd << endl;
//        cerr << "[[ READ." << endl;
        int cc = read( fd, buf, 1024 );
        // cerr << "cc=" << cc << " buf=" << buf << endl;
        Value ret(cc);
        ret.setBinary( buf, cc );
        this->out("out")->write( ret );
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



REGISTER_NODE(UDPSrc)