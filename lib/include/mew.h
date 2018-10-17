#ifndef MEW_H
#define MEW_H
#pragma once

#include <map>

#include <poll.h>

#include <cppbackports/any.h>

#include "blockingconcurrentqueue.h"
#include "channelqueue.h"
#include "jobworker.h"
#include "timer.h"
#include "safe_ptr.h"
#include "selectio.h"
#include "uvtimer.h"

#include <unistd.h>

#include <cxxabi.h>
const char* demangle(const char* name);

#include <uv.h>

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
        IOReference * ioref;
    } IOTickInstance;

    typedef struct
    {
        size_t expected_type;
        std::function<void(cpp::any)> f;
        moodycamel::ConcurrentQueue< cpp::any > queue;
        std::mutex lock;
        Mew* context;
        std::atomic<int> jobCpt;
    } SubscriptionReference;

    typedef struct
    {
        SubscriptionReference * sref;
        cpp::any item;
    } SubscriptionPublication;

    typedef struct
    {
        void* sub_backend;
        Mew* context;
        // moodycamel::BlockingConcurrentQueue< cpp::any > queue;
        ChannelQueue< cpp::any > queue;
        std::function<void(cpp::any)> f;
        std::function<void(mew::Mew*,void*)> fc;
        std::string sub_topic;
    } ChannelReference;

public:
    Mew()
    {

        // UV - experimental
        uv_loop_init( &_loop );

        _pollScheduled.store(0);
        _poller = new SelectIO();
        _ioThread = std::thread([&]{
            while(true)
            {
                this->processIO();
            }
        });

        _minTimerInterval = 0.001;
        _numAdditional = std::thread::hardware_concurrency() - 1;
        // cerr << "Number of additionnal threads = " << _numAdditional << endl;
         _numAdditional = 0;
        _scheduler = std::make_shared<mew::JobScheduler>( _numAdditional );
        if( _numAdditional == 0 )
        {
            _minTimerInterval = 0.001;
        }
    }

    virtual ~Mew()
    {

    }

    void run()
    {
        Job * timerJob = createUVLoopJob();
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
        UVTimer * uvt = new UVTimer( &_loop, this, f, dt_sec );
        _uvtimers.push_back( uvt );

        /*
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
        */


    }

    template<typename R, typename Arg>
    void * io( R (*fptr)(Mew*, Arg), int filedescriptor )
    {
        cerr << "io fptr" << endl;
        std::function<R(Mew*, Arg)> f = static_cast<std::function<R(Mew*, Arg)> >(fptr);
        return this->io( f, filedescriptor );
    }

    template<typename R, typename Arg>
    void * io( std::function<R(Mew*, Arg)> f, int filedescriptor )
    {
        // Job * ioJob = createIOCheckJob();
        // _scheduler->push( ioJob );
        cerr << "io std::function FD=" << filedescriptor << endl;
        IOReference * ioref = new IOReference();
        ioref->fd = filedescriptor;
        ioref->f = f;
        ioref->processed = 0;
        ioref->context = this;
        _ioRefs.push_back( ioref );
        _poller->add( filedescriptor );
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
        cerr << "SUB template<typename R, typename ...Args>" << endl;
        cerr << "SUB tyepid=" << typeid(Arg).name() << endl;
        SubscriptionReference* sref = new SubscriptionReference();
        sref->expected_type = typeid(Arg).hash_code();
        sref->context = this;
        sref->jobCpt = 0;

        {
            sref->f = [f, this](cpp::any aobj){
//                cerr << "SUB received type: " << demangle(aobj.type().name()) << endl;
                Arg couille0;
                try{
                    couille0 = cpp::any_cast<Arg>( aobj );
                }
                catch ( cpp::bad_any_cast& e )
                {
                    cerr << "SUB BAD template<typename R, typename ...Args>" << endl;
                    cerr << "SUB BAD tyepid=" << demangle( typeid(Arg).name() ) << endl;
                    cerr << "SUB bad_any_cast " << endl;
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
                sref->jobCpt++;
                SubscriptionPublication * subpub = new SubscriptionPublication();
                subpub->sref = sref;
                subpub->item = aobj;
                Job * j = new Job( []( Job* j ){
                        SubscriptionPublication * spub = (SubscriptionPublication*)(j->userData());
                        spub->sref->f( spub->item );
                        spub->sref->jobCpt--;
                        delete spub;
                }, subpub);
                j->label() = "JOB_PUBLICATION";
                _scheduler->push( j );

                /*
                cpp::any aobj = obj;
                {
                    sref->f( aobj );
                }
                */

            }
        }
        else
        {
            cerr << "Could not find topic \"" << topic << "\" to push to ." << endl;
        }
    }

    void printSubscriptions();

    // Channels
    template<typename R, typename Arg>
    void * channel_open( const std::string& topic, R(*fptr)(Mew*, Arg) )
    {
        std::function<R(Mew*, Arg)> f = static_cast<std::function<R(Mew*, Arg)> >(fptr);
        return channel_open( topic, f );
    }

    template<typename R, typename Arg, typename std::enable_if<!std::is_same<Arg, cpp::any>::value, Arg>::type* = nullptr>
    void * channel_open( const std::string& channel_name, std::function<R(Mew*, Arg)> f )
    {
        ChannelReference * cref = new ChannelReference;
        cref->f = [f, this](cpp::any aobj){
            cerr << "CHANNEL received type: " << demangle(aobj.type().name()) << endl;
            Arg couille0;
            try{
                couille0 = cpp::any_cast<Arg>( aobj );
            }
            catch ( cpp::bad_any_cast& e )
            {
                cerr << "CHANNEL BAD template<typename R, typename ...Args>" << endl;
                cerr << "CHANNEL BAD tyepid=" << demangle( typeid(Arg).name() ) << endl;
                cerr << "CHANNEL bad_any_cast " << endl;
                cerr << e.what() << endl;
                return;
            }
//            cerr << "will call f_channel" << endl;
            f(this, couille0);
//            cerr << "called f_channel" << endl;
        };

        // Create a subscription topic for the receiving end...
        cref->fc = [cref](mew::Mew* m, void* nope){
            cpp::any aa;
            if( cref->queue.pop(aa) )
            {
//                cerr << "call_0" << endl;
                cref->f( aa );
                cerr << "call_1" << endl;
            }
            else
            {
                cerr << "rate." << endl;
            }
        };
        cref->sub_topic = channel_name + "";
        cref->sub_backend = (void*)(subscribe( cref->sub_topic, cref->fc ));
        _channels.insert( make_pair( channel_name, cref ) );
        //

        return cref;
    }

    template<typename R, typename Arg, typename std::enable_if<std::is_same<Arg, cpp::any>::value, Arg>::type* = nullptr>
    void * channel_open( const std::string& channel_name, std::function<R(Mew*, Arg)> f )
    {
        ChannelReference * cref = new ChannelReference;

        cref->f = [f, this](cpp::any aobj){
            // cerr << "any direct" << endl;
            f(this, aobj);
        };

        // Create a subscription topic for the receiving end...
        cref->fc = [cref](mew::Mew* m, void* nope){
            cpp::any aa;
            if( cref->queue.pop(aa) )
            {
                // cerr << "call_0" << endl;
                cref->f( aa );
//                cerr << "call_1" << endl;
            }
            else
            {
                cerr << "encore rate." << endl;
            }
        };
        cref->sub_topic = channel_name + "";
        cref->sub_backend = (void*)(subscribe( cref->sub_topic, cref->fc ));
        _channels.insert( make_pair( channel_name, cref ) );
        //

        return cref;
    }

    template<typename T>
    void channel_write( const std::string& channel_name, T&& obj )
    {
        // Try to find channel definition
        ChannelReference * cref = nullptr;
        if( _channels.find( channel_name ) != _channels.end() )
        {
            cref = _channels[ channel_name ];
        }
        else
        {
            cerr << "uh nope." << endl;
        }

        if( cref )
        {
            cpp::any pany = obj;
            if( cref->queue.push( pany ) )
            {
                this->publish( cref->sub_topic, (void*)(0) );
            }
            else
            {
                cerr << "CHANNEL " << cref->sub_topic << " could not enqueue !" << endl;
            }
        }
        else
        {
            cerr << "lol nope. " << channel_name << endl;
        }

    }
    //

    void set_timer_interval( void* timer_ref, double dt_secs );

    std::shared_ptr< JobScheduler > scheduler()
    {
        return _scheduler;
    }

private:
    // Job Scheduler
    std::shared_ptr< JobScheduler > _scheduler;
    int _numAdditional;

    // UV - experimental
    uv_loop_t _loop;
    Job *createUVLoopJob();

    // UV - timers
    std::vector< UVTimer* > _uvtimers;

    // Timer
    std::mutex _timerLock;
    std::vector< TimerReference* > _timerRefs;
    std::vector< TimerReference* > processTimers();
    Job * createTimerCheckJob();
    double _minTimerInterval;

    // IO
    std::thread _ioThread;
    std::atomic<int> _pollScheduled;
    IOPoller * _poller;
    std::mutex _ioLock;
    std::vector< IOReference* > _ioRefs;
    double _ioTimeoutSecs;
    void processIO();
    Job * createIOCheckJob();

    // PUB/SUB
    std::map< std::string, std::vector< SubscriptionReference* > > _subscriptions;
    std::mutex _subRegistryMtx;
    void processSubscriber( SubscriptionReference* sref );

    // CHANNELS
    std::map< std::string, ChannelReference* > _channels;

protected:

};

mew::Mew* mew_init( int argc, char** argv );

}

#endif
