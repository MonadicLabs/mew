#pragma once

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <chrono>

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

    virtual void tick();

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
            // cerr << "t_elapsed=" << tref.t.elapsed() << endl;
            if( tref.t.elapsed() >= tref.dt_usec )
            {
                Task * tt = new TimerTask( tref.f );
                _taskQueue.enqueue( tt );
                tref.t.reset();
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
            Worker::tick();
        }

        /*
        Task * t;
        while(true)
        {
            // Wait for IO tasks
            if( _ioQueue.try_dequeue(t) )
            {
                // Do something
                switch( t->type() )
                {
                case Task::TIMER:
                {
                    break;
                }

                default:
                    break;
                }

                // Destroy task
                delete t;
            }

            Worker::tick();
        }
        */

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
    void timer( std::function<R(Arg)> f, double dt_usec )
    {
        cerr << "timer std::function" << endl;
        TimerReference tref;
        tref.dt_usec = dt_usec;
        tref.f = f;
        f( 12345 );
        tref.t.reset();
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
        Timer t;
        double dt_usec;
        std::function<void(double)> f;
    } TimerReference;

    std::map< std::string, std::vector< SubscriptionReference > > _subscriptions;
    std::vector< TimerReference > _timerRefs;

    // Task queues
    moodycamel::BlockingConcurrentQueue<mew::Task*> _taskQueue;
//    moodycamel::BlockingConcurrentQueue<mew::Task*> _ioQueue;

    // Workers
    std::vector< Worker * > _workers;

    // Global timer
    Timer _globalTimer;

};

void Worker::tick()
{
    Task * t = nullptr;
    _parent->_taskQueue.wait_dequeue(t);
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

        default:
            break;
        }

        delete t;
    }
}

}
