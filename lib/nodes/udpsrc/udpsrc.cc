
#include "udpsrc.h"
#include "workspace.h"

mew::UDPSrc::UDPSrc(WorkSpace *ctx)
    :Node(ctx)
{
    declare_output( "out" );
    declare_parameter( "port", Value::NUMBER, 5000 );
}

mew::UDPSrc::~UDPSrc()
{

}

void mew::UDPSrc::onContextChange(mew::WorkSpace *ctx)
{
    int ufd = 0;
    mew::Mew * m = ctx->getRuntime();
    std::function<void(mew::Mew*,int)> f = [this](mew::Mew*, int fd){
        char buf[1024];
        // cerr << "fd=" << fd << endl;
        int cc = read( fd, buf, 1024 );
        // cerr << "cc=" << cc << " buf=" << buf << endl;
        Value ret(cc);
        ret.setBinary( buf, cc );
        this->out("out")->write( ret );
    };
    void * popo = m->io( f, ufd );
}

void mew::UDPSrc::onSetParameter(const string &paramName, Value value)
{
    cerr << "onSetParameter ! " << endl;
}



REGISTER_NODE(UDPSrc)
