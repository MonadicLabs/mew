#pragma once

/*
#include "mewconfig.h"

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <chrono>
#include <limits>
#include <sstream>

#include <poll.h>

#include "singleton.h"
#include "cppbackports/any.h"
#include "runnable.h"
#include "task.h"
#include "timer.h"

#include "blockingconcurrentqueue.h"
#include "cuckoohash_map.hh"

#ifdef MEW_USE_PROFILING
#include "Remotery.h"
#endif

#include "job.h"

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
#ifdef MEW_USE_PROFILING
        std::stringstream sstr;
        sstr << "mew_worker_" << this;
        rmt_SetCurrentThreadName( sstr.str().c_str() );
#endif
        while(!hasToStop())
        {
            tick();
        }
    }

    virtual void tick( double timeout_s = -1 );

private:
    mew::Mew * _parent;

};

class Mew : public Worker
{

    //    friend class singleton<Mew>;
    friend class mew::Worker;

public:

    Mew( size_t additionalWorkerNum = 0 )
        :Worker(this), _numWorkers(additionalWorkerNum), _ioTimeoutSecs(0.005), _lastIOPoll(0.0)
    {

#ifdef MEW_USE_PROFILING
        Remotery* rmt;
        int remoteryPort;
        while( rmt_CreateGlobalInstance(&rmt) != RMT_ERROR_NONE )
        {
            rmtSettings* settings = rmt_Settings();
            remoteryPort = 1234 + rand() % 1024;
            settings->port = remoteryPort;
        }
        rmtSettings* settings = rmt_Settings();
        remoteryPort = settings->port;
        cerr << "Remotery PORT=" << remoteryPort << endl;
        sleep(3);
#endif

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
                //                double latencyRatio = fabs((tref.t.elapsed() - tref.dt_sec) / tref.dt_sec);
                //                //                cerr << "latencyRatio" << latencyRatio << endl;
                //                if( latencyRatio >= 0.2 )
                //                {
                //                    cerr << "t_elapsed=" << tref.t.elapsed() << "requested: " << tref.dt_sec << endl;
                //                    cerr << "/!\\ latency_ratio=" << latencyRatio << endl;
                //                }
                //                cerr << "appapap=" << approach << endl;
                Task * tt = new TimerTask( tref.f, globalTime() );
                pushTask( tt, 0 );
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
            if( !ioref.locked )
            {
                pfds.push_back({ ioref.fd, POLLIN, 0} );
                _callbacks.insert( make_pair( ioref.fd, ioref.f ) );
            }
        }
        int ret = ::poll(&pfds[0], pfds.size(), _ioTimeoutSecs * 1000.0 );
        if(ret < 0){
            // TODO
            // throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
            cerr << "poll ERRROR!" << endl;
        }
        else if( ret == 0 )
        {
            //            cerr << "timeout !" << endl;
        }
        else
        {
            for( struct pollfd &p : pfds)
            {
                if(p.revents == POLLIN)
                {
                    //                    cerr << "data ready ?" << endl;
                    p.revents = 0;
                    lockIO( p.fd );
                    pushTask( new IOTask( _callbacks[ p.fd ], p.fd ) );
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
            //            if( _numWorkers == 0 )
            //            {
            //                Worker::tick( min(_minTimerValue / 10.0, _ioTimeoutSecs / 2.0) );
            //            }
            double dt = (_minTimerValue * 1000000.0) / 32.0;
            usleep( dt );
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
                pushTask( t );
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
        ioref.locked = false;
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
        bool locked;
    } IOSubscriptionReference;

    typedef struct
    {
        Timer t;
        double dt_sec;
        double approach;
        std::function<void(double)> f;
    } TimerReference;

    std::map< std::string, std::vector< SubscriptionReference > > _subscriptions;

    // Tasks are put in queues depending on their priorities
    cuckoohash_map< int, moodycamel::BlockingConcurrentQueue<mew::Task*>* > _tasks;
    std::mutex _taskMtx;
    mew::Task* fetchTask( int timeout_s = -1 )
    {
        cerr << "timeout_s=" << timeout_s << endl;
        mew::Task* ret = nullptr;
        size_t maxval = 0;
        std::vector< int > keys;
        {
            auto lt = _tasks.lock_table();
            for (const auto &it : lt) {
                keys.push_back( it.first );
            }
        }

        for( int p : keys )
        {
            moodycamel::BlockingConcurrentQueue<mew::Task*> * taskQueue = _tasks.find(p);
            if( taskQueue->size_approx() > 0 )
            {
                taskQueue->wait_dequeue(ret);
            }
        }

        return ret;
    }
    void pushTask( mew::Task* t, int priority  = 1 )
    {
        if( !_tasks.contains( priority ) )
        {
            moodycamel::BlockingConcurrentQueue<mew::Task*> * taskQueue = new moodycamel::BlockingConcurrentQueue<mew::Task*>();
            _tasks.insert( priority, taskQueue );
        }
        moodycamel::BlockingConcurrentQueue<mew::Task*> * taskQueue = _tasks.find( priority );
        taskQueue->enqueue( t );
    }

    // Workers
    int _numWorkers;
    std::vector< Worker * > _workers;

    // Timers & Global timer
    Timer _globalTimer;
    std::vector< TimerReference > _timerRefs;
    double _minTimerValue;

    // IO
    std::vector< IOSubscriptionReference > _ioRefs;
    void unlockIO( int fd )
    {
        for( IOSubscriptionReference& ioref : _ioRefs )
        {
            if( ioref.fd == fd )
            {
                ioref.locked = false;
            }
        }
    }
    void lockIO( int fd )
    {
        for( IOSubscriptionReference& ioref : _ioRefs )
        {
            if( ioref.fd == fd )
            {
                ioref.locked = true;
            }
        }
    }
    double _ioTimeoutSecs;
    double _lastIOPoll;

};

void Worker::tick(double timeout_s)
{

#ifdef MEW_USE_PROFILING
    rmt_BeginCPUSample(TICK, 0);
#endif

    Task * t = nullptr;

    Timer dt;
    dt.start();
    t = _parent->fetchTask();
    dt.stop();
    cerr << "worker_proc_time=" << dt.elapsed() << endl;
    //    sleep(1);

    //    cerr << "WORKER " << this << " tick_time=" << _parent->globalTime() << endl;

    //    dt.start();
    //    _parent->_taskQueue.try_dequeue( t );
    //    dt.stop();
    //    cerr << "dequeue time=" << dt.elapsed() << endl;


    if( t )
    {
        //        cerr << "****** w_" << this << " - Processing task TYPE=" << t->type() << endl;

        switch( t->type() )
        {

        case Task::DATA:
        {
#ifdef MEW_USE_PROFILING
            rmt_BeginCPUSample(TASK_DATA, 0);
#endif
            DataTask * dt = (DataTask*)t;
            dt->_f( dt->_data );
#ifdef MEW_USE_PROFILING
            rmt_EndCPUSample();
#endif
            break;
        }

        case Task::TIMER:
        {
#ifdef MEW_USE_PROFILING
            rmt_BeginCPUSample(TASK_TIMER, 0);
#endif
            TimerTask * tt = (TimerTask*)t;
            double globalTime = _parent->globalTime();
            double execdt = globalTime - tt->_triggerTime;
            //            cerr << "w=" << this << "global=" << globalTime << " trigger=" << tt->_triggerTime << " - exec_dt=" << execdt << endl;
            tt->_f( tt->_triggerTime );
#ifdef MEW_USE_PROFILING
            rmt_EndCPUSample();
#endif
            break;
        }

        case Task::IO:
        {
#ifdef MEW_USE_PROFILING
            rmt_BeginCPUSample(TASK_IO, 0);
#endif
            IOTask * iot = (IOTask*)t;
            cerr << "calling io." << endl;
            iot->_f( iot->_fd );
            _parent->unlockIO( iot->_fd );
#ifdef MEW_USE_PROFILING
            rmt_EndCPUSample();
#endif
            break;
        }

        case Task::IO_POLL:
        {
#ifdef MEW_USE_PROFILING
            rmt_BeginCPUSample(TASK_IO_POLL, 0);
#endif
            _parent->processIO();
#ifdef MEW_USE_PROFILING
            rmt_EndCPUSample();
#endif
            break;
        }

        default:
            break;
        }

        delete t;
    }

#ifdef MEW_USE_PROFILING
    rmt_EndCPUSample();
#endif

}

}

#ifdef MEW_USE_PROFILING
#include "Remotery.c"
#endif
*/
