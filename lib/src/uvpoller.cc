
#include "uvpoller.h"
#include "mew.h"

void mew::UVPoller::periodic()
{
#ifdef MEW_USE_PROFILING
        rmt_BeginCPUSampleDynamic( "IO_POLL_DIRECT", 0);
#endif

    mew::Job * popo = new Job( []( mew::Job* j ) {
            UVPoller* pref = (UVPoller*)(j->userData());
            pref->exec();
    }, this );
    popo->label() = "IO_POLL_EXECUTION";
    _context->scheduler()->push( popo );

//        exec();
#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
}

void mew::UVPoller::exec()
{
    this->_f( _context, _fd );
}
