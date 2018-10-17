
#include <node.h>

#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

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

    // LOOPBACK TEST
    int _lo;
    struct sockaddr_in serveraddr;
    //

protected:

};

}
