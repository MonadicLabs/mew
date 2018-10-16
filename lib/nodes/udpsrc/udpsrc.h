
#include <node.h>

namespace mew {

class UDPSrc : public Node
{
public:
    UDPSrc( WorkSpace* ctx = nullptr );
    virtual ~UDPSrc();

    virtual void onContextChange(WorkSpace *ctx);
    virtual void onSetParameter(const string &paramName, Value value);

private:
    void init_udp_socket();
    int _fd;
    int setNonBlocking( int fd );

protected:

};

}
