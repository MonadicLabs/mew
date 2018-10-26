
#include <iostream>
#include <iomanip>
#include <thread>
#include <sstream>
using namespace std;

#include <unistd.h>

#include <mew.h>
#include <workstealingqueue_impl2.h>
#include <jobworker.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include "node.h"
#include "graph.h"
#include "value.h"
#include "connection.h"

// IO
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
//

#include <cppbackports/any.h>

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

void sub1( mew::Mew* ctx, double popo )
{
    cerr << "msg=" << popo << endl;
    // std::string str = msg.string();
    // std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    // ctx->publish( "capital", str );
    burn_cpu(1e2);
}

void sub2( mew::Mew* ctx, std::string msg )
{
    cerr << "upper_cased: " << msg << endl;
    //    ctx->publish( "popo", msg );
}

int cpt=0;
void timer_func1( mew::Mew* ctx, double dt_usec )
{
    cerr << "timer_func1 dt=" << std::setprecision(20) << dt_usec << endl;
    burn_cpu(1e4);
}

void timer_func2( mew::Mew* ctx, double dt_usec )
{
    cerr << "timer_func2 dt=" << dt_usec << endl;
    ctx->publish( "in0", Value(std::string("polbak")) );
    // ctx->publish( "in0", Value( 260986.1234 ) );
    burn_cpu(1e2);
}

void io_test1( mew::Mew* ctx, int fd )
{
    //    cerr << "cocou" << endl;
    char buf[1024];
    int rr = read( fd, buf, 1024 );
    stringstream sstr;
    sstr << "read " << rr << " bytes " << " from fd=" << fd << endl;
    // ctx->publish( "popo", 12345.45 );
    // burn_cpu(1e6);
    cerr << sstr.str();
}

class TestNode : public mew::Node
{
public:
    TestNode( mew::WorkSpace* ctx, mew::Graph* parent )
        :Node( ctx, parent )
    {
        declare_input( "in0" );
        declare_output( "out0" );
        declare_parameter( "width", Value::NUMBER, 800 );
        // setTickInterval( 1 );
    }

    virtual ~TestNode()
    {

    }

    virtual void onTimer(double dt)
    {
        cerr << ".. TICK " << this << " dt=" << dt << endl;
        out("out0")->write( std::string("popo") );
    }

    virtual void onInput( const std::string& portName, cpp::any& v )
    {
        cerr << this->str_id() << " received data on port " << portName << endl;
        Value content;
        try{
            content = cpp::any_cast<Value>( v );
            cerr << "CONTENT=" << content.str() << endl;
        }
        catch ( cpp::bad_any_cast& e )
        {
            cerr << "BAD template<typename R, typename ...Args>" << endl;
        }
    }
};

int main( int argc, char** argv )
{

    /*
    mew::Mew * m = new mew::Mew();

    // UDP TEST
#define BUFLEN 2048
#define NPACK 10

    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        //diep("socket");
        cerr << ":( socket." << endl;

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(9940);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(s, (const sockaddr*)(&si_me), sizeof(si_me))==-1)
        cerr << ":( bind" << endl;
    m->io( io_test1, s );
    //

//    m->timer( timer_func1, 0.1 );
//    m->timer( timer_func1, 0.1 );
//    m->timer( timer_func1, 0.001 );
//    m->timer( timer_func1, 0.001 );

    //    m->channel_open( "chan0", sub1 );

    m->run();
    */

    mew::WorkSpace * ws = new mew::WorkSpace();
    mew::Graph * g = ws->createEmptyGraph();

    mew::Node * tn = mew::Node::create("UDPSrc");
    tn->setParameter( "port", 9940 );
    tn->setContext( ws );
    tn->setParameter( "rate", 0.0001 );
    mew::Node * tn2 = mew::Node::create("UDPSink");
        tn2->setParameter("host", "127.0.0.1");
        tn2->setParameter("port", 5000);
    tn2->setContext( ws );
    g->addNode( tn );
    g->addNode( tn2 );
    cerr << "tn id=" << tn->str_id() << endl;
    cerr << "tn2 id=" << tn2->str_id() << endl;
    g->addConnection( tn->out("out"), tn2->in("in") );
    cerr << "this is a test" << endl;

    mew::Node * tn_ = mew::Node::create("Clock");
    tn_->setParameter( "port", 9941 );
    tn_->setContext( ws );
    tn_->setParameter( "rate", 0.001 );
    mew::Node * tn2_ = mew::Node::create("Console");
        tn2_->setParameter("host", "127.0.0.1");
        tn2_->setParameter("port", 6000);
    tn2_->setContext( ws );
    g->addNode( tn_ );
    g->addNode( tn2_ );
    cerr << "tn id=" << tn_->str_id() << endl;
    cerr << "tn2 id=" << tn2_->str_id() << endl;
    g->addConnection( tn_->out("out"), tn2_->in("in") );
    cerr << "this is a test again" << endl;

    ws->run();

    /*
    std::deque< cpp::any > anyqueue;
    for( int i = 0; i < 1000000; ++i )
    {
        if( i % 2 == 0 )
        {
            anyqueue.push_back( 1234.0 );
        }
        else
        {
            anyqueue.push_back( std::string("coucou") );
        }
    }
    cerr << "done queue " << endl;

    int udpPort = 9940 + rand() % 1000;
    cerr << "udpPort=" << udpPort << endl;

    mew::Mew m;
    double dt = 0.001;
    for( int k = 0; k < 1; ++k )
    {
        m.timer( timer_func2, 0.001 );
    }
    m.subscribe( "popo", sub1 );
    //    m.subscribe( "capital", sub2 );

    //    m.timer( timer_func1, dt );
    //    m.timer( timer_func1, dt * 2.0 );
    //    m.timer( timer_func1, dt * 4.0 );

    // UDP TEST
#define BUFLEN 512
#define NPACK 10

    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        //diep("socket");
        cerr << ":( socket." << endl;

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(udpPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(s, (const sockaddr*)(&si_me), sizeof(si_me))==-1)
        cerr << ":( bind" << endl;
    m.io( io_test1, s );
    //

    std::thread popo([&](){
        while(true)
        {
            sleep(1);
            mew::Graph g( &m );
            TestNode popo( &m, &g );
            TestNode popo2( &m, &g );
            mew::Connection conn( popo.out("out0"), popo2.in("in0") );
            sleep(1);
        }
    });

    sleep(1);

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
    */

    return 0;

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
