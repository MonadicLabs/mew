
#include <iostream>
#include <thread>
#include <sstream>
using namespace std;

#include <unistd.h>

#include <mew.h>


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
}

void test_timer( double gt )
{
    cerr << "lkgjkdfgjdklgj" << endl;
    cerr << "tick t=" << gt << endl;
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

int main( int argc, char** argv )
{
    mew::Mew m( 0 );
    m.subscribe( "popo", popo1 );
    m.timer( test_timer, 0.01 );

    mew::Mew* mm = &m;

    thread worker = thread{[mm](){
        int k = 0;
        while(true)
        {
            mm->push( "popo", k++ );
            usleep(1500);
        }
    }};

    m.run();

}
