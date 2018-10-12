
#include <node.h>

#include <netinet/in.h>

namespace mew {

class UDPSink : public Node
{
public:
    UDPSink( WorkSpace* ctx = nullptr );
    virtual ~UDPSink();

    virtual void onContextChange(WorkSpace *ctx);
    virtual void onSetParameter(const string &paramName, Value value);
    virtual void onInput(const string &portName, any &v);

private:
    void init_udp_socket();
    int _fd;
    struct sockaddr_in serveraddr;

protected:

};

}
