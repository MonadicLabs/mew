#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <chrono>
#include <limits>

#include <poll.h>

#include "singleton.h"
#include "any.h"
#include "runnable.h"
#include "task.h"
#include "timer.h"
#include "blockingconcurrentqueue.h"

namespace mew
{

class Mew;
class Worker : public Runnable
{
public:
    Worker( Mew* m )
        :_parent(m)
    {
        cerr << "Worker. Reporting in !" << endl;
    }

    virtual ~Worker()
    {

    }

    virtual void run()
    {
        while(!hasToStop())
        {
            tick();
        }
    }

    virtual void tick( int64_t timeout_s = 0 );

private:
    mew::Mew * _parent;

};

class Mew : public Worker
{

    //    friend class singleton<Mew>;
    friend class mew::Worker;

public:

    Mew( size_t additionalWorkerNum = 0 )
        :Worker(this)
    {
        _minTimerValue = std::numeric_limits<double>::max();
        for( size_t i = 0; i < additionalWorkerNum; ++i )
        {
            // Add a new worker.
            Worker * w = new Worker(this);
            _workers.push_back(w);
        }
    }

    virtual ~Mew()
    {

    }

    void processTimers()
    {
        for( TimerReference& tref : _timerRefs )
        {
            if( tref.t.elapsed() > tref.dt_sec )
            {
                double latencyRatio = fabs((tref.t.elapsed() - tref.dt_sec) / tref.dt_sec);
//                cerr << "latencyRatio" << latencyRatio << endl;
                if( latencyRatio >= 0.2 )
                {
                    cerr << "t_elapsed=" << tref.t.elapsed() << "requested: " << tref.dt_sec << endl;
                    cerr << "/!\\ latency_ratio=" << latencyRatio << endl;
                }
//                cerr << "appapap=" << approach << endl;
                Task * tt = new TimerTask( tref.f, globalTime() );
                _taskQueue.enqueue( tt );
                tref.t.reset();
            }
        }
    }

    void processIO()
    {
        std::vector<struct pollfd> pfds;
        std::map< int, std::function<void(int)> > _callbacks;
        for( IOSubscriptionReference& ioref : _ioRefs )
        {
            pfds.push_back({ ioref.fd, POLLIN, 0} );
            _callbacks.insert( make_pair( ioref.fd, ioref.f ) );
        }
        int ret = ::poll(&pfds[0], pfds.size(), 1000);
        if(ret < 0){
            // TODO
            // throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
            cerr << "poll ERRROR!" << endl;
        }
        else if( ret == 0 )
        {
            cerr << "timeout !" << endl;
        }
        else
        {
            for( struct pollfd &p : pfds)
            {
                if(p.revents == POLLIN)
                {
                    cerr << "data ready ?" << endl;
                    p.revents = 0;
                    _taskQueue.enqueue( new IOTask( _callbacks[ p.fd ], p.fd ) );
                }

            }
        }
    }

    virtual void run(){

        _globalTimer.reset();

        // If any workers available, start them.
        for( Worker* w : _workers )
        {
            w->start();
        }

        while(true)
        {
            processTimers();
//            if( _minTimerValue >= 0.005 )
//            {
//                Worker::tick( _minTimerValue / 100.0 );
//            }
//            else
//            {
//                usleep(10);
//            }
        }

    }

    template<typename R, typename Arg>
    void subscribe( const std::string& topic, R (*fptr)(Arg) )
    {
        std::function<R(Arg)> f = static_cast<std::function<R(Arg)> >(fptr);
        subscribe( topic, f );
    }

    template<typename R, typename Arg>
    void subscribe( const std::string& topic, std::function<R(Arg)> f )
    {
        cerr << "template<typename R, typename ...Args>" << endl;
        //        cerr << "tyepid=" << typeid(Arg).name() << endl;
        SubscriptionReference sref;
        sref.expected_type = typeid(Arg).hash_code();
        sref.f = [f](cpp::any aobj){
            Arg couille0;
            try{
                couille0 = cpp::any_cast<Arg>( aobj );
            }
            catch ( cpp::bad_any_cast& e )
            {
                return;
            }
            f(couille0);
        };

        if( _subscriptions.find( topic ) == _subscriptions.end() )
        {
            std::vector< SubscriptionReference > srefs;
            _subscriptions.insert( std::make_pair(topic, srefs) );
        }

        {
            auto& sl = _subscriptions[ topic ];
            sl.push_back( sref );
        }
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

        if( _minTimerValue > dt_sec )
        {
            _minTimerValue = dt_sec;
        }

        _timerRefs.push_back( tref );
    }

    template<typename T>
    void push( const std::string& topic, T&& obj )
    {
        cpp::any aobj;
        aobj = obj;

        // Look for subscriber
        if( _subscriptions.find( topic ) != _subscriptions.end() )
        {
            auto& sl = _subscriptions[ topic ];
            for( SubscriptionReference& sref : sl )
            {
                // Add a data task
                Task * t = new DataTask( sref.f, aobj );
                _taskQueue.enqueue( t );
            }
        }
        else
        {
            cerr << "Could not find topic \"" << topic << "\" to push to ." << endl;
        }
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
        IOSubscriptionReference ioref;
        ioref.fd = filedescriptor;
        ioref.f = f;
        _ioRefs.push_back( ioref );
    }

    double globalTime()
    {
        return _globalTimer.elapsed();
    }

private:

    typedef struct
    {
        size_t expected_type;
        std::function<void(cpp::any)> f;
    } SubscriptionReference;

    typedef struct
    {
        std::function<void(int)> f;
        int fd;
    } IOSubscriptionReference;

    typedef struct
    {
        Timer t;
        double dt_sec;
        double approach;
        std::function<void(double)> f;
    } TimerReference;

    std::map< std::string, std::vector< SubscriptionReference > > _subscriptions;
    std::vector< IOSubscriptionReference > _ioRefs;

    // Task queues
    moodycamel::BlockingConcurrentQueue<mew::Task*> _taskQueue;
    //    moodycamel::BlockingConcurrentQueue<mew::Task*> _ioQueue;

    // Workers
    std::vector< Worker * > _workers;

    // Timers & Global timer
    Timer _globalTimer;
    std::vector< TimerReference > _timerRefs;
    double _minTimerValue;


};

void Worker::tick(int64_t timeout_s)
{
    Task * t = nullptr;

    if( timeout_s > 0 )
        _parent->_taskQueue.wait_dequeue(t);
    else
        _parent->_taskQueue.wait_dequeue_timed(t, timeout_s * 1000000);

    if( t )
    {
        switch( t->type() )
        {

        case Task::DATA:
        {
            DataTask * dt = (DataTask*)t;
            dt->_f( dt->_data );
            break;
        }

        case Task::TIMER:
        {
            TimerTask * tt = (TimerTask*)t;
            double globalTime = _parent->globalTime();
            tt->_f( globalTime );
            break;
        }

        case Task::IO:
        {
            IOTask * iot = (IOTask*)t;
            cerr << "calling io." << endl;
            iot->_f( iot->_fd );
            break;
        }

        default:
            break;
        }

        delete t;
    }
}

}
