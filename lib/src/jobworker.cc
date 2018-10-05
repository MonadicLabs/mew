
#include <sstream>
using namespace std;

#include "jobworker.h"
#include "jobscheduler.h"

void mew::JobWorker::run()
{
#ifdef MEW_USE_PROFILING
    std::stringstream sstr;
    sstr << "mew_worker_" << this;
    rmt_SetCurrentThreadName( sstr.str().c_str() );
#endif
    while(true)
    {
        Job* j = nullptr;
        if( pop(j) )
        {
            j->run( this );
            delete j;
        }
        else
        {
            // try to steal
            if( _parentScheduler )
            {
                //                cerr << "parent=" << _parentScheduler << endl;
                if( _parentScheduler->trySteal( j, this ) )
                {
                    j->run(this);
                    delete j;
                }
                else
                {
                    std::this_thread::sleep_for( std::chrono::microseconds(100) );
                }
            }
            std::this_thread::sleep_for( std::chrono::microseconds(100) );
        }
    }
}
