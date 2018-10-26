
#include "uvasync.h"
#include "mew.h"

void mew::UVAsync::periodic()
{
//#ifdef MEW_USE_PROFILING
//        rmt_BeginCPUSampleDynamic( "ASYNC_DIRECT", 0);
//#endif

    mew::Job * popo = new Job( []( mew::Job* j ) {
            UVPoller* pref = (UVPoller*)(j->userData());
            pref->exec();
    }, this );
    popo->label() = "ASYNC_POPO_EXECUTION";
    _context->scheduler()->push( popo );

//    exec();

//#ifdef MEW_USE_PROFILING
//        rmt_EndCPUSample();
//#endif
}

void mew::UVAsync::exec()
{
    this->_f( _context );
}
