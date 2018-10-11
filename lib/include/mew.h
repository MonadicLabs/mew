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

#include <unistd.h>

#include <cxxabi.h>
const char* demangle(const char* name);

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

    typedef struct
    {
        size_t expected_type;
        std::function<void(cpp::any)> f;
        moodycamel::ConcurrentQueue< cpp::any > queue;
        std::mutex lock;
        Mew* context;
        std::atomic<int> jobCpt;
    } SubscriptionReference;

public:
    Mew()
    {
        _minTimerInterval = 1.0;
        int numAdditionnalThreads = std::thread::hardware_concurrency() - 1;
        // cerr << "Number of additionnal threads = " << numAdditionnalThreads << endl;
//        numAdditionnalThreads = 0;
        _scheduler = std::make_shared<mew::JobScheduler>( numAdditionnalThreads );
    }

    virtual ~Mew()
    {

    }

    void run()
    {
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
        if( tref->dt_sec < _minTimerInterval )
            _minTimerInterval = tref->dt_sec;
        return (void*)tref;
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

    template<typename R, typename Arg, typename std::enable_if<!std::is_same<Arg, cpp::any>::value, Arg>::type* = nullptr>
    void * subscribe( const std::string& topic, std::function<R(Mew*, Arg)> f )
    {
        std::unique_lock< std::mutex >( _subRegistryMtx );
        cerr << "template<typename R, typename ...Args>" << endl;
        cerr << "tyepid=" << typeid(Arg).name() << endl;
        SubscriptionReference* sref = new SubscriptionReference();
        sref->expected_type = typeid(Arg).hash_code();
        sref->context = this;
        sref->jobCpt = 0;

        {
            sref->f = [f, this](cpp::any aobj){
                cerr << "received type: " << demangle(aobj.type().name()) << endl;
                Arg couille0;
                try{
                    couille0 = cpp::any_cast<Arg>( aobj );
                }
                catch ( cpp::bad_any_cast& e )
                {
                    cerr << "BAD template<typename R, typename ...Args>" << endl;
                    cerr << "BAD tyepid=" << demangle( typeid(Arg).name() ) << endl;
                    cerr << "bad_any_cast " << endl;
                    cerr << e.what() << endl;
                    return;
                }
                f(this, couille0);
            };
        }

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

    template<typename R, typename Arg, typename std::enable_if<std::is_same<Arg, cpp::any>::value, Arg>::type* = nullptr>
    void * subscribe( const std::string& topic, std::function<R(Mew*, Arg)> f )
    {
        std::unique_lock< std::mutex >( _subRegistryMtx );
        cerr << "template<typename R, typename ...Args>" << endl;
        cerr << "tyepid=" << typeid(cpp::any).name() << endl;
        SubscriptionReference* sref = new SubscriptionReference();
        sref->expected_type = typeid(cpp::any).hash_code();
        sref->context = this;
        sref->jobCpt = 0;

        sref->f = [f, this](cpp::any aobj){
            f(this, aobj);
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

    bool unsubscribe( void* subReference );

    template<typename T>
    void publish( const std::string& topic, T&& obj )
    {
        // cerr << "**** TOPIC=" << topic << endl;
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

    void printSubscriptions();
    void set_timer_interval( void* timer_ref, double dt_secs );

private:
    // Job Scheduler
    std::shared_ptr< JobScheduler > _scheduler;

    // Timer
    std::mutex _timerLock;
    std::vector< TimerReference* > _timerRefs;
    std::vector< TimerReference* > processTimers();
    Job * createTimerCheckJob();
    double _minTimerInterval;

    // IO
    std::mutex _ioLock;
    std::vector< IOReference* > _ioRefs;
    double _ioTimeoutSecs;
    void processIO();
    Job * createIOCheckJob();

    // PUB/SUB
    std::map< std::string, std::vector< SubscriptionReference* > > _subscriptions;
    std::mutex _subRegistryMtx;
    void processSubscriber( SubscriptionReference* sref );

protected:

};

mew::Mew* mew_init( int argc, char** argv );

}

#endif
