
#include "mew.h"
#include "uvtimer.h"
#include "job.h"

bool mew::Mew::unsubscribe(void *subReference)
{
    std::unique_lock< std::mutex >( _subRegistryMtx );
    bool removed = false;
    for( auto& kv : _subscriptions )
    {
        int idx = -1;
        for( int k = 0; k < kv.second.size(); ++k )
        {
            if( kv.second[ k ] == subReference )
            {
                idx = k;
                removed = true;
                break;
            }
        }
        if( idx >= 0 )
        {
            cerr << "idx=" << idx << endl;
            cerr << "key=" << kv.first << " P_SUBS=" << kv.second.size() << endl;
            SubscriptionReference* csubref = (SubscriptionReference*)subReference;
            kv.second.erase( kv.second.begin() + idx );
            // Wait for jobs completion
            while( csubref->jobCpt != 0 )
            {
                usleep(1000);
            }
            delete csubref;
            //
            cerr << "key=" << kv.first << " P_SUBS=" << kv.second.size() << endl;
        }
        if( removed )
            break;
    }
    cerr << "SUBSCRIPTIONS=" << _subscriptions.size() << endl;
    return removed;
}

void mew::Mew::printSubscriptions()
{
    std::unique_lock< std::mutex >( _subRegistryMtx );
    for( auto& kv : _subscriptions )
    {
        cerr << "topic: " << kv.first << endl;
        for( int k = 0; k < kv.second.size(); ++k )
        {
            cerr << "\t\t sref=" << kv.second[ k ] << endl;
        }
        cerr << endl;
    }
}

void mew::Mew::set_timer_interval(void *timer_ref, double dt_secs)
{
    for( TimerReference* tref : _timerRefs )
    {
        if( tref == timer_ref )
        {
            tref->dt_sec = dt_secs;
            if( tref->dt_sec < _minTimerInterval )
            {
                _minTimerInterval = tref->dt_sec;
            }
        }
    }
}

mew::Job *mew::Mew::createUVLoopJob()
{
    if( _numAdditional > 0 )
    {
        Job * uvJob = new Job( []( Job* j ){
                mew::Mew * m = (mew::Mew*)j->userData();
                uv_run( &(m->_loop), UV_RUN_NOWAIT);
                Job * nextJob = m->createUVLoopJob();
                m->scheduler()->push( nextJob );
        }, this );
        uvJob->label() = "UV_UPDATE";
        return uvJob;
    }
    else
    {
        Job * uvJob = new Job( []( Job* j ){
                mew::Mew * m = (mew::Mew*)j->userData();
                uv_run( &(m->_loop), UV_RUN_DEFAULT);
                Job * nextJob = m->createUVLoopJob();
                m->scheduler()->push( nextJob );
        }, this );
        uvJob->label() = "UV_UPDATE";
        return uvJob;
    }
}

std::vector<mew::Mew::TimerReference *> mew::Mew::processTimers()
{
    std::vector< TimerReference* > ret;
    _timerLock.lock();
    for( int i = 0; i < _timerRefs.size(); ++i )
    {
        TimerReference* tref = _timerRefs[i];
        if( tref->processed == 0 && tref->t.elapsed() > tref->dt_sec )
        {
            ret.push_back(tref);
        }
    }
    _timerLock.unlock();
    return ret;
}

mew::Job *mew::Mew::createTimerCheckJob()
{
    Job * timerJob = new Job( []( Job* j ){
            mew::Mew * m = (mew::Mew*)j->userData();
            std::vector< TimerReference* > trigList = m->processTimers();
            for( TimerReference* tref : trigList )
    {
            tref->processed = 1;
            mew::Job * trigJob = new Job( []( mew::Job* j ) {
            TimerReference * tr = (TimerReference*)j->userData();
            tr->f( tr->context, tr->t.elapsed() );
            tr->t.reset();
            tr->processed = 0;
}, tref );
    trigJob->label() = "TIMER_CALLBACK";
    j->pushChild( trigJob );
}

usleep( (int)((m->_minTimerInterval / 10.0) * 1000000.0) );

// Should be half the smallest timer tick value
j->pushChild( m->createTimerCheckJob() );
}, this );
timerJob->label() = "TIMER_CHECK";
return timerJob;
}

void mew::Mew::processIO()
{
    std::vector< int > rfd = _poller->poll( 0.0 );
    for( int fd : rfd )
    {
        for( IOReference* ioref : _ioRefs )
        {
            if( ioref->fd == fd )
            {
                ioref->processed = 1;
                ioref->f( ioref->context, fd );
                ioref->processed = 0;

                //                ioref->processed = 1;
                //                mew::Job * ioJob = new Job( []( mew::Job* j ) {
                //                        IOReference * ioref = (IOReference*)j->userData();
                //                        ioref->f( ioref->context, ioref->fd );
                //                        ioref->processed = 0;
                //                }, ioref );
                //                ioJob->label() = "IO_PROCESS";
                //                _scheduler->push( ioJob );

            }
        }
    }
}

mew::Job *mew::Mew::createIOCheckJob()
{
    Job * ioJob = new Job( []( Job* j ){
            mew::Mew * m = (mew::Mew*)j->userData();
            m->processIO();
            m->_pollScheduled.store( m->_pollScheduled.load() - 1 );
            if( m->_pollScheduled.load() <= 0 )
    {
            j->pushChild( m->createIOCheckJob() );
}
}, this );
    ioJob->label() = "IO_CHECK";
    _pollScheduled.store( _pollScheduled.load() + 1 );
    return ioJob;
}

void mew::Mew::processSubscriber(mew::Mew::SubscriptionReference *sref)
{
    cpp::any aobj;
    if( sref->queue.try_dequeue( aobj ) )
    {
        cerr << "." << endl;
        sref->f( aobj );
    }
}

mew::Mew *mew::mew_init(int argc, char **argv)
{
    return new mew::Mew();
}

#include <cxxabi.h>
const char *demangle(const char *name)
{
    static char buf[1024];
    size_t size = sizeof(buf);
    int status;
    // todo:
    char* res = abi::__cxa_demangle (name,
                                     buf,
                                     &size,
                                     &status);
    buf[sizeof(buf) - 1] = 0; // I'd hope __cxa_demangle does this when the name is huge, but just in case.
    return res;
}
