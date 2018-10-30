
#include "uvtimer.h"
#include "jobscheduler.h"
#include "job.h"
#include "mew.h"

#include <iostream>
using namespace std;

void mew::UVTimer::periodic()
{
#ifdef MEW_USE_PROFILING
        rmt_BeginCPUSampleDynamic( "TIMER_JOB_CREATION", 0);
#endif
        /*
    mew::Job * popo = new Job( []( mew::Job* j ) {
            UVTimer* tref = (UVTimer*)(j->userData());
            tref->exec();
    }, this );
    popo->label() = "TIMER_EXECUTION";
    _context->scheduler()->push( popo );
        */

        cerr << "lol timeer." << endl;

#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
}

void mew::UVTimer::exec()
{
    double gt = (double)uv_hrtime() / (double)(1000.0*1000.0*1000.0);
    this->_f( _context, gt );
}
