#pragma once

#include <map>

#include <poll.h>

#include "jobworker.h"
#include "timer.h"

namespace mew
{

class Mew
{

    typedef struct
    {
        Timer t;
        double dt_sec;
        double approach;
        std::function<void(double)> f;
        std::atomic< int > processed;
    } TimerReference;

    typedef struct
    {
        std::function<void(int)> f;
        int fd;
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
        Job * ioJob = createIOCheckJob();
        _scheduler->push( ioJob );
        _scheduler->push( timerJob );
        return _scheduler->run();
    }

    template<typename R, typename Arg>
    void timer( R (*fptr)(Arg), double dt_usec )
    {
        cerr << "timer fptr" << endl;
        std::function<R(Arg)> f = static_cast<std::function<R(Arg)> >(fptr);
        this->timer( f, dt_usec );
    }

    template<typename R, typename Arg>
    void timer( std::function<R(Arg)> f, double dt_sec )
    {
        cerr << "timer std::function" << endl;
        TimerReference* tref = new TimerReference();
        tref->dt_sec = dt_sec;
        tref->approach = std::numeric_limits<double>::max();
        tref->f = f;
        tref->t.reset();
        tref->processed = 0;
        _timerRefs.push_back( tref );
    }

    template<typename R, typename Arg>
    void io( R (*fptr)(Arg), int filedescriptor )
    {
        cerr << "io fptr" << endl;
        std::function<R(Arg)> f = static_cast<std::function<R(Arg)> >(fptr);
        this->io( f, filedescriptor );
    }

    template<typename R, typename Arg>
    void io( std::function<R(Arg)> f, int filedescriptor )
    {
        cerr << "io std::function" << endl;
        IOReference * ioref = new IOReference();
        ioref->fd = filedescriptor;
        ioref->f = f;
        ioref->processed = 0;
        _ioRefs.push_back( ioref );
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
            if( tref->t.elapsed() > tref->dt_sec && tref->processed == 0 )
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
                j->pushChild( m->createTimerCheckJob() );
                std::vector< TimerReference* > trigList = m->processTimers();
                for( TimerReference* tref : trigList )
                {
                    tref->processed = 1;
                    mew::Job * trigJob = new Job( []( mew::Job* j ) {
                        TimerReference * tr = (TimerReference*)j->userData();
                        tr->f( tr->t.elapsed() );
                        tr->processed = 0;
                        tr->t.reset();
                    }, tref );
                    trigJob->label() = "TIMER_CALLBACK";
                    j->pushChild( trigJob );
                }
                // Should be half the smallest timer tick value
                usleep(100);
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
    _ioLock.lock();
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
    int ret = ::poll(&pfds[0], pfds.size(), 1.0 );
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
                _callbacks[ p.fd ]->f( p.fd );
#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
            }
        }
    }
    _ioLock.unlock();
}
Job * createIOCheckJob()
{
    Job * ioJob = new Job( []( Job* j ){
            mew::Mew * m = (mew::Mew*)j->userData();
            j->pushChild( m->createIOCheckJob() );
            // cerr << "dlkfjdlkfjdfklj" << endl;
            m->processIO();
            // usleep(100);
    }, this );
    ioJob->label() = "IO_CHECK";
    return ioJob;
}

protected:

};
}
