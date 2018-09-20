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

        std::shared_ptr<mew::Job> timerJob = std::make_shared<mew::Job>( []( std::shared_ptr<mew::Job> j ){
                mew::Mew * m = (mew::Mew*)j->userData();
                m->processTimers();
                usleep(10);
                j->pushChild( j );
    }, this );

        _scheduler->push( timerJob );

    }

private:
    // Job Scheduler
    std::shared_ptr< JobScheduler > _scheduler;

    // Timers
    std::vector< TimerReference > _timerRefs;
    void processTimers()
    {
        for( TimerReference& tref : _timerRefs )
        {
//            cerr << "tref.elapsed=" << tref.t.elapsed() << endl;
            if( tref.t.elapsed() > tref.dt_sec )
            {
                tref.f( tref.t.elapsed() );
                /*
                auto cbf = tref.f;
                cerr << "cbf=" << &tref << endl;
                std::shared_ptr< mew::Job > jexec = std::make_shared< mew::Job >( []( std::shared_ptr<mew::Job> j ){
                    TimerReference * tr = (TimerReference*)(j->userData() );
                    cerr << "f=" << tr << endl;
                    tr->f( 55.0 );
                    cerr << "executing Timer callback somehow !" << endl;
                }, &tref );
                _scheduler->push( jexec );
                */
                tref.t.reset();
            }
        }

    }

protected:

};
}
