#pragma once

#include "iopoller.h"

#include <sys/select.h>

#include <set>
#include <iostream>
#include <atomic>

using namespace std;

namespace mew
{
class SelectIO : public IOPoller
{
public:
    SelectIO()
    {
        FD_ZERO(&readfds);
        _polling = 0;
    }

    virtual ~SelectIO()
    {

    }

    virtual void add( int fd )
    {
        FD_SET(fd, &readfds);
        _fds.insert( fd );
        update_n();
    }

    virtual void remove( int fd )
    {
        FD_SET(fd, &readfds);
        _fds.erase( fd );
        update_n();
    }

    virtual std::vector< int > poll( double timeout_sec )
    {
        std::vector< int > ret;

//        int k = 0;
//        for( int fd : _fds )
//        {
//            cerr << "fd[" << k << "] = " << fd << endl;
//            k++;
//        }

        if( _fds.size() == 0 )
        {
//            cerr << "IO_NOPE" << endl;
            usleep(100);
            return ret;
        }
        else
        {
            FD_ZERO(&readfds);
            for( int fd : _fds )
            {
                FD_SET(fd, &readfds);
            }
            update_n();
        }

        // _polling = 1;
//        cerr << "POLL !!!!! cpt=" << _fds.size() << endl;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000000;
        int rv = select(n, &readfds, NULL, NULL, &tv);
        if (rv == -1) {
//            printf(" ERROR : select \n"); // error occurred in select()
        } else if (rv == 0) {
//            printf("Timeout occurred!  No data after 10.5 seconds.\n");
        } else {

            for( int fd : _fds )
            {
                if (FD_ISSET(fd, &readfds))
                {
                    ret.push_back( fd );
                }
            }
//            cerr << ret.size() << " fds ready to be read." << endl;
        }
        // _polling = 0;
        return ret;
    }

private:
    fd_set readfds;
    std::set< int > _fds;
    int n;

    std::atomic<int> _polling;

    void update_n()
    {
        n = 0;
        for( int fd : _fds )
        {
            if( fd > n )
                n = fd;
        }
        n++;
    }

};
}
