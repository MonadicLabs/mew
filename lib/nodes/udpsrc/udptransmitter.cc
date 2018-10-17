
#include "udptransmitter.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
using namespace std;

#define UDP_MTU 1500


UDPTransmitter::UDPTransmitter(uint16_t port, std::string host, std::string multicastInterface)
{
    if (( _fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {

    }

    struct sockaddr_in groupSock;
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(_host.c_str());
    groupSock.sin_port = htons(_port);

    /*
    if( isMulticastAddress( _host ) )
    {
        struct in_addr localInterface;
        std::string intIp = getNetworkInterfaceIP( _multicastInterface );
        localInterface.s_addr = inet_addr(intIp.c_str());
        if(setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
        {
            NOVALINK_LOG(ERROR) << "setsockopt Multicast failed on interface " << _multicastInterface << " IP_ADDR=" << intIp;
        }
        else
        {
            NOVALINK_LOG(INFO) << "Setting multicast to " << _host << " on interface " << _multicastInterface << " : OK - IP_ADDR = " << intIp;
        }
    }
    else
    {

    }
    */

}

UDPTransmitter::~UDPTransmitter()
{
    if( _fd >= 0 ){
        close(_fd);
    }
}

void UDPTransmitter::send(uint8_t *data, uint32_t data_len)
{
    struct sockaddr_in groupSock;

    int s, i, slen=sizeof(groupSock);
    char buf[8192];

    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;

    // if( isMulticastAddress( _host ) )
    groupSock.sin_addr.s_addr = inet_addr( _host.c_str() );
    groupSock.sin_port = htons(_port);
    if (inet_aton(_host.c_str(), &groupSock.sin_addr)==0) {

    }

    if( data_len > UDP_MTU )
    {
#ifdef USE_LOGGING
      NOVALINK_LOG(WARNING) << "Data packet size (" << data_len << ") is superior to UDP MTU (" << UDP_MTU << ")";
#endif
    }

    if (sendto(_fd, data, data_len, 0, (const struct sockaddr*)(&groupSock), slen)==-1)
    {

    }
}
