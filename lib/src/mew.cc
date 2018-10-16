
#include "mew.h"

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
    std::vector<struct pollfd> pfds;
    std::map< int, IOReference* > _callbacks;
    for( IOReference* ioref : _ioRefs )
    {
        // if( ioref->processed == 0 )
        {
            pfds.push_back({ ioref->fd, POLLIN, 0} );
            _callbacks.insert( make_pair( ioref->fd, ioref ) );
        }
    }
//    cerr << "pfds.size=" << pfds.size() << endl;
    int ret = ::poll(&pfds[0], pfds.size(), 0 );
    if(ret < 0){
        // TODO
        // throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
        cerr << "poll ERRROR!" << endl;
    }
    else if( ret == 0 )
    {
        // cerr << "timeout !" << endl;
    }
    else
    {
        for( struct pollfd &p : pfds)
        {
            if(p.revents == POLLIN)
            {
                p.revents = 0;

                //#ifdef MEW_USE_PROFILING
                //                rmt_BeginCPUSample( IO_CALLBACK, 0);
                //#endif

                _callbacks[ p.fd ]->processed = 1;
                _callbacks[ p.fd ]->f( _callbacks[ p.fd ]->context, p.fd );
                _callbacks[ p.fd ]->processed = 0;

//                _callbacks[ p.fd ]->processed = 1;
//                mew::Job * ioJob = new Job( []( mew::Job* j ) {
//                        IOReference * ioref = (IOReference*)j->userData();
//                        ioref->f( ioref->context, ioref->fd );
//                        ioref->processed = 0;
//                }, _callbacks[ p.fd ] );
//                ioJob->label() = "IO_PROCESS";
//                _scheduler->push( ioJob );

                //#ifdef MEW_USE_PROFILING
                //                rmt_EndCPUSample();
                //#endif
            }
        }
    }
}

mew::Job *mew::Mew::createIOCheckJob()
{
    Job * ioJob = new Job( []( Job* j ){
            mew::Mew * m = (mew::Mew*)j->userData();
            // cerr << "dlkfjdlkfjdfklj" << endl;
            m->processIO();
            usleep(1000);
            j->pushChild( m->createIOCheckJob() );
    }, this );
    ioJob->label() = "IO_CHECK";
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
