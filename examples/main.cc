
#include <iostream>
#include <thread>
#include <sstream>
using namespace std;

#include <unistd.h>

#include <mew.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

void popo1( int q )
{
    cerr << "popo1=" << q << endl;
}

typedef struct
{
    std::string str;
} test_struct;

void popo2( test_struct str )
{
    cerr << "popo2=" << endl;
    usleep(1000);
}

void test_timer( double gt )
{
    cerr << "***** tick t=" << gt << endl;
    double popo = exp( (double)rand() / (double)RAND_MAX );
}

/*
class MewTest
{
public:
    MewTest()
    {
        std::string topicName = "popo";
        cerr << "mew." << endl;
        mew::Mew * m = mew::Mew::get_instance();

        std::function<void(int)> binding = std::bind(&MewTest::mt1, this, std::placeholders::_1);
        std::function<void(test_struct)> binding2 = std::bind(&MewTest::mt2, this, std::placeholders::_1);
        m->subscribe( topicName, binding );
        m->subscribe( topicName, binding2 );

        for( int k = 0; k < 100; ++k )
        {
            if( k % 2 == 0 )
            {
                m->push( topicName, (int)k );
            }
            else
            {
                test_struct ts;
                std::stringstream sstr;
                sstr << "K=" << k;
                ts.str = sstr.str();
                m->push( topicName, ts );
            }
        }
    }

    void threadtest()
    {
        // std::thread th( &MewTest::mt0, this );
    }

private:
    void mt0()
    {

    }

    void mt1( int k )
    {
        cerr << "mt1=" << k << endl;
    }

    void mt2( test_struct str )
    {
        cerr << "mt2=" << str.str << endl;
    }


protected:

};
*/

void process_io( int fd )
{
    char buf[1024];
    int rr = read( fd, buf, 1024 );
    cerr << "rr=" << rr << " bytes" << endl;
}

int main( int argc, char** argv )
{

    srand(time(NULL));

    mew::Mew m( 1 );
//    m.subscribe( "popo", popo1 );
    m.timer( test_timer, 1.0 );
    mew::Mew* mm = &m;

    // UDP TEST
#define BUFLEN 512
#define NPACK 10
#define PORT 9940
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        //diep("socket");
        cerr << ":( socket." << endl;

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(s, (const sockaddr*)(&si_me), sizeof(si_me))==-1)
        cerr << ":( bind" << endl;

//    m.io( process_io, s );
    //

    thread worker = thread{[mm](){
        int k = 0;
        while(true)
        {
//            mm->push( "popo", k++ );
            usleep(15000);
        }
    }};

    m.run();

}
