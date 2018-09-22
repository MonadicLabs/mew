#pragma once

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
    } TimerReference;

public:
    Mew()
    {
        _scheduler = std::make_shared<mew::JobScheduler>();
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
        TimerReference tref;
        tref.dt_sec = dt_sec;
        tref.approach = std::numeric_limits<double>::max();
        tref.f = f;
        tref.t.reset();
        _timerRefs.push_back( tref );
    }

    void print()
    {
        _scheduler->print();
    }

private:
    // Job Scheduler
    std::shared_ptr< JobScheduler > _scheduler;

    // Timers
    typedef struct
    {
        Mew * mew;
        TimerReference ref;
    } TimerTrigger;
    std::mutex _timerLock;
    std::vector< TimerReference > _timerRefs;
    std::vector< TimerReference > processTimers()
    {
        std::vector< TimerReference > ret;
        _timerLock.lock();
        for( int i = 0; i < _timerRefs.size(); ++i )
        {
            TimerReference& tref = _timerRefs[i];
            if( tref.t.elapsed() > tref.dt_sec )
            {
                // cerr << "trigger_add " << i << endl;
                ret.push_back(tref);
                tref.t.reset();
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
                std::vector< TimerReference > trigList = m->processTimers();
                for( TimerReference tref : trigList )
                {
                    /*
                    // IT LEAKS...
                    TimerTrigger * trig = new TimerTrigger();
                    trig->mew = m;
                    trig->ref = tref;
                    mew::Job * trigJob = new Job( []( mew::Job* j ) {
                        TimerTrigger * t = (TimerTrigger*)j->userData();
                        t->ref.f( 777.0 );
                        delete t;
                    }, trig );
                    trigJob->label() = "TIMER_CALLBACK";
                    j->pushChild( trigJob );
                    */
        #ifdef MEW_USE_PROFILING
                rmt_BeginCPUSample( TIMER_CALLBACK, 0);
        #endif
                    tref.f(555.0);
        #ifdef MEW_USE_PROFILING
                rmt_EndCPUSample();
        #endif
                    tref.t.reset();
                }
                usleep(10);
        }, this );
        timerJob->label() = "TIMER_CHECK";
        return timerJob;
    }

protected:

};
}
