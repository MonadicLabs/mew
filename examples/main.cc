
#include <iostream>
#include <thread>
#include <sstream>
using namespace std;

#include <unistd.h>

#include <mew2.h>
#include <workstealingqueue_impl2.h>
#include <jobworker.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>


// IO
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

//

void burn_cpu( int iter = 1e7 )
{
    float f1;
    float f2 = 2;
    float f3 = 3;
    for( int i =0 ; i < iter; i++)
    {
        f1 = (i * f2 + i / f3) * 0.5; //or divide by 2.0f, respectively
    }
}

/*
void job_func1( mew::Job* j );
void job_func2( mew::Job* j )
{
    for( int k = 0; k < 20; ++k )
    {
        j->pushChild( new mew::Job( job_func1 ) );
    }
    //    cerr << "job_func2" << endl;
    float f1;
    float f2 = 2;
    float f3 = 3;
    for( int i =0 ; i < 1e5; i++)
    {
        f1 = (i * f2 + i / f3) * 0.5; //or divide by 2.0f, respectively
    }
}

void job_func1( mew::Job* j )
{
    //    cerr << "job_func1" << endl;
    float f1;
    float f2 = 2;
    float f3 = 3;
    for( int i =0 ; i < 1e5; i++)
    {
        f1 = (i * f2 + i / f3) * 0.5; //or divide by 2.0f, respectively
    }
    for( int k = 0; k < 10; ++k )
    {
        j->pushChild( new mew::Job( job_func2 ) );
    }
}
*/

void timer_func1( double dt_usec )
{
    cerr << "timer_func1 dt=" << dt_usec << endl;
    //    usleep(100000);
    // burn_cpu(1e3);
}

void timer_func2( double dt_usec )
{
    cerr << "timer_func2 dt=" << dt_usec << endl;
    // burn_cpu(1e6);
}

void io_test1( int fd )
{
//    cerr << "cocou" << endl;
    char buf[1024];
    int rr = read( fd, buf, 1024 );
    cerr << "read " << rr << "bytes " << endl;
    // burn_cpu(1e6);
}

int main( int argc, char** argv )
{

    //    WorkStealingStack< std::shared_ptr< int > > test;
    //    test.Push( std::make_shared<int>() );

    //    std::atomic< std::shared_ptr< int > > testa;
    //    testa.store( std::make_shared<int>() );
    //        mew::Job j;

    //        sleep(5);

    mew::Mew m;
    double dt = 0.1;
    for( int k = 0; k < 1; ++k )
    {
        m.timer( timer_func2, dt );
    }
    m.timer( timer_func1, 0.1 );

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
    m.io( io_test1, s );
    //

    m.run();

    //    srand( time(NULL) );
    //    //    mew::Job j( job_func1 );
    //    std::vector< mew::Job* > jobs;
    //    for( int k= 0; k < 10; ++k )
    //    {
    //        mew::Job * popo = new mew::Job( []( mew::Job* j ){
    //                j->test();
    //        });
    //        jobs.push_back( popo );
    //    }

    //    for( mew::Job* j : jobs )
    //    {
    //        j->run();
    //    }

    //    return 0;

    //    mew::JobScheduler sched;
    //    sched.push( new mew::Job( job_func1 ) );
    //    sched.run();

    //    return 0;
}

/*
#include <functional>
#include <stdio.h>
#include <iostream>

template<typename F>
void AFunctionThatTakesLambdaFunctionAsArgument( F&& lambda )
{
    std::string hello("hello world");
    lambda( hello );
}

void lambdaExample1()
{
    std::string prefix = "Method1 Using std::function ";
    std::function<void(std::string)>lambda = [prefix] (std::string message) -> int
    {
        std::cout << prefix.c_str() << ": " << message.c_str() << "\n";
        return 0;
    };
    AFunctionThatTakesLambdaFunctionAsArgument( lambda );
}

void lambdaExample2()
{
    typedef int (*MyLambda)(std::string );
    MyLambda lambda  = [] (std::string message) -> int
    {
        std::cout << std::string("Method2 Using typedef function ").c_str() <<
": " << message.c_str() << "\n";
        return 0;
    };
    AFunctionThatTakesLambdaFunctionAsArgument( lambda );
}

void lambdaExample3()
{
    struct MyClosure
    {
        std::string m_prefix;
        int operator() (std::string message)
        {
            std::cout << m_prefix.c_str() << ": " << message.c_str() << "\n";
            return 0;
        }
    };
    MyClosure lambda;
    lambda.m_prefix = "Method3 Using struct ";//capture data
    AFunctionThatTakesLambdaFunctionAsArgument( lambda );
}

void lambdaExamples()
{
    lambdaExample1();
    lambdaExample2();
    lambdaExample3();
}
int main(int , char **)
{
    lambdaExamples();
    return 0;
}
*/
