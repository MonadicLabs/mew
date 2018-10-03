#ifndef MEW_H
#define MEW_H
#pragma once

#include <map>

#include <poll.h>

#include <cppbackports/any.h>

#include "blockingconcurrentqueue.h"
#include "jobworker.h"
#include "timer.h"
#include "safe_ptr.h"

namespace mew
{

class Mew
{
    typedef struct
    {
        Mew* context;
        Timer t;
        double dt_sec;
        double approach;
        std::function<void(Mew*, double)> f;
        std::atomic< int > processed;
    } TimerReference;

    typedef struct
    {
        std::function<void(Mew*, int)> f;
        int fd;
        Mew* context;
        std::atomic<int> processed;
    } IOReference;

public:
    Mew()
    {
        int numAdditionnalThreads = std::thread::hardware_concurrency() - 1;
        //                int numAdditionnalThreads = 0;
        cerr << "Number of additionnal threads = " << numAdditionnalThreads << endl;
        _scheduler = std::make_shared<mew::JobScheduler>( numAdditionnalThreads );
    }

    virtual ~Mew()
    {

    }

    void run()
    {
        //
        Job * timerJob = createTimerCheckJob();
        _scheduler->push( timerJob );
        return _scheduler->run();
    }

    template<typename R, typename Arg>
    void * timer( R (*fptr)(Mew*, Arg), double dt_usec )
    {
        cerr << "timer fptr" << endl;
        std::function<R(Mew*, Arg)> f = static_cast<std::function<R(Mew*, Arg)> >(fptr);
        return this->timer( f, dt_usec );
    }

    template<typename R, typename Arg>
    void * timer( std::function<R(Mew*, Arg)> f, double dt_sec )
    {
        cerr << "timer std::function" << endl;
        TimerReference* tref = new TimerReference();
        tref->dt_sec = dt_sec;
        tref->approach = std::numeric_limits<double>::max();
        tref->f = f;
        tref->t.reset();
        tref->processed = 0;
        tref->context = this;
        _timerRefs.push_back( tref );
        return (void*)tref;
    }

    void set_timer_interval( void* timer_ref, double dt_secs )
    {
        for( TimerReference* tref : _timerRefs )
        {
            if( tref == timer_ref )
            {
                tref->dt_sec = dt_secs;
            }
        }
    }

    template<typename R, typename Arg>
    void * io( R (*fptr)(Mew*, Arg), int filedescriptor )
    {
        cerr << "io fptr" << endl;
        std::function<R(Mew*, Arg)> f = static_cast<std::function<R(Mew*, Arg)> >(fptr);
        return this->io( f, filedescriptor );
    }

    template<typename R, typename Arg>
    void  * io( std::function<R(Mew*, Arg)> f, int filedescriptor )
    {
        Job * ioJob = createIOCheckJob();
        _scheduler->push( ioJob );
        cerr << "io std::function" << endl;
        IOReference * ioref = new IOReference();
        ioref->fd = filedescriptor;
        ioref->f = f;
        ioref->processed = 0;
        ioref->context = this;
        _ioRefs.push_back( ioref );
        return (void*)ioref;
    }

    template<typename R, typename Arg>
    void * subscribe( const std::string& topic, R(*fptr)(Mew*, Arg) )
    {
        std::function<R(Mew*, Arg)> f = static_cast<std::function<R(Mew*, Arg)> >(fptr);
        return subscribe( topic, f );
    }

    template<typename R, typename Arg>
    void * subscribe( const std::string& topic, std::function<R(Mew*, Arg)> f )
    {
        std::unique_lock< std::mutex >( _subRegistryMtx );
        cerr << "template<typename R, typename ...Args>" << endl;
        cerr << "tyepid=" << typeid(Arg).name() << endl;
        SubscriptionReference* sref = new SubscriptionReference();
        sref->expected_type = typeid(Arg).hash_code();
        sref->context = this;
        sref->jobCpt = 0;
        sref->f = [f, this](cpp::any aobj){
            Arg couille0;
            try{
                couille0 = cpp::any_cast<Arg>( aobj );
            }
            catch ( cpp::bad_any_cast& e )
            {
                //                cerr << "bad_any_cast" << endl;
                //                cerr << e.what() << endl;
                return;
            }
            f(this, couille0);
        };

        if( _subscriptions.find( topic ) == _subscriptions.end() )
        {
            std::vector< SubscriptionReference* > srefs;
            _subscriptions.insert( std::make_pair(topic, srefs) );
        }

        {
            auto& sl = _subscriptions[ topic ];
            sl.push_back( sref );
        }

        return (void*)sref;

    }

    bool unsubscribe( void* subReference )
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

    template<typename T>
    void publish( const std::string& topic, T&& obj )
    {
        std::unique_lock< std::mutex >( _subRegistryMtx );
        // Look for subscriber
        if( _subscriptions.find( topic ) != _subscriptions.end() )
        {
            auto& sl = _subscriptions[ topic ];
            for( SubscriptionReference* sref : sl )
            {
                cpp::any aobj = obj;
                sref->queue.enqueue( aobj );
                sref->jobCpt++;
                Job * j = new Job( []( Job* j ){
                        SubscriptionReference * sref = (SubscriptionReference*)(j->userData());
                        cpp::any pol;
                        if( sref->queue.try_dequeue( pol ) )
                        {
                            sref->f( pol );
                        }
                        sref->jobCpt--;
                }, sref);
                j->label() = "JOB_SUB_TICK";
                _scheduler->push( j );
            }
        }
        else
        {
            cerr << "Could not find topic \"" << topic << "\" to push to ." << endl;
        }
    }

    void printSubscriptions()
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

private:
    // Job Scheduler
    std::shared_ptr< JobScheduler > _scheduler;
    std::mutex _timerLock;
    std::vector< TimerReference* > _timerRefs;
    std::vector< TimerReference* > processTimers()
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
    Job * createTimerCheckJob()
    {
        Job * timerJob = new Job( []( Job* j ){
                mew::Mew * m = (mew::Mew*)j->userData();
                std::vector< TimerReference* > trigList = m->processTimers();
                for( TimerReference* tref : trigList )
        {

                /*
                                                    tref->processed = 1;
                                                    tref->f( tref->context, tref->t.elapsed() );
                                                    tref->t.reset();
                                                    tref->processed = 0;
                                                */

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
    usleep(500);
    // Should be half the smallest timer tick value
    j->pushChild( m->createTimerCheckJob() );
}, this );
timerJob->label() = "TIMER_CHECK";
return timerJob;
}

// IO
std::mutex _ioLock;
std::vector< IOReference* > _ioRefs;
double _ioTimeoutSecs;
void processIO()
{
    //    _ioLock.lock();
    //     cerr << "processIO" << endl;
    std::vector<struct pollfd> pfds;
    std::map< int, IOReference* > _callbacks;
    for( IOReference* ioref : _ioRefs )
    {
        //        if( ioref->processed == 0 )
        {
            pfds.push_back({ ioref->fd, POLLIN, 0} );
            _callbacks.insert( make_pair( ioref->fd, ioref ) );
        }
    }
    int ret = ::poll(&pfds[0], pfds.size(), 500.0 );
    if(ret < 0){
        // TODO
        // throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
        //        cerr << "poll ERRROR!" << endl;
    }
    else if( ret == 0 )
    {
        //                    cerr << "timeout !" << endl;
    }
    else
    {
        for( struct pollfd &p : pfds)
        {
            if(p.revents == POLLIN)
            {
                p.revents = 0;
#ifdef MEW_USE_PROFILING
                rmt_BeginCPUSample( IO_CALLBACK, 0);
#endif
                _callbacks[ p.fd ]->f( _callbacks[ p.fd ]->context, p.fd );

                /*
                Job * ioJob = new Job( []( Job* j ){
                    IOReference* ioref = (IOReference*)j->userData();
                    ioref->f( ioref->context, ioref->fd );
                }, _callbacks[ p.fd ] );
*/

#ifdef MEW_USE_PROFILING
                rmt_EndCPUSample();
#endif
                // this->_scheduler->push( ioJob );

            }
        }
    }
    //    _ioLock.unlock();
}
Job * createIOCheckJob()
{
    Job * ioJob = new Job( []( Job* j ){
            mew::Mew * m = (mew::Mew*)j->userData();
            // j->pushChild( m->createIOCheckJob() );
            // cerr << "dlkfjdlkfjdfklj" << endl;
            m->processIO();
            j->pushChild( m->createIOCheckJob() );
}, this );
    ioJob->label() = "IO_CHECK";
    return ioJob;
}

// PUB/SUB
typedef struct
{
    size_t expected_type;
    std::function<void(cpp::any)> f;
    moodycamel::ConcurrentQueue< cpp::any > queue;
    std::mutex lock;
    Mew* context;
    std::atomic<int> jobCpt;
} SubscriptionReference;
std::map< std::string, std::vector< SubscriptionReference* > > _subscriptions;
std::mutex _subRegistryMtx;
void processSubscriber( SubscriptionReference* sref )
{
    cpp::any aobj;
    if( sref->queue.try_dequeue( aobj ) )
    {
        cerr << "." << endl;
        sref->f( aobj );
    }
}
//

protected:

};

mew::Mew* mew_init( int argc, char** argv )
{
    return new mew::Mew();
}

}

#endif
