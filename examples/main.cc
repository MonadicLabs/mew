
#include <iostream>
#include <thread>
#include <sstream>
using namespace std;

#include <unistd.h>

#include <mew2.h>

#include <wsdeque.h>
#include <jobworker.h>


#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

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
}

int main( int argc, char** argv )
{
        mew::Mew m;
        m.timer( timer_func1, 0.0001 );

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
