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

#include "cds_job.h"
#include "uvtimer.h"
#include "uvpoller.h"
#include "uvasync.h"

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
        UVAsync * async_ref;
        Mew* context;
        ChannelQueue< cpp::any > queue;
        std::function<void(cpp::any)> f;
        std::function<void(mew::Mew*)> fc;
        std::string sub_topic;
    } ChannelReference;

public:
    Mew();

    virtual ~Mew()
    {

    }

    void run();

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
        return uvt;
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
        cerr << "io std::function FD=" << filedescriptor << endl;
        UVPoller * uvp = new UVPoller( &_loop, this, f, filedescriptor );
        _uvpollers.push_back( uvp );
        return uvp;
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
        /*
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
            }
        }
        else
        {
            cerr << "Could not find topic \"" << topic << "\" to push to ." << endl;
        }
        */
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

        cref->async_ref = new UVAsync( &_loop, this, f );
        cref->sub_topic = channel_name + "";
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
        cref->fc = [cref](mew::Mew* m){
            cpp::any aa;
            if( cref->queue.pop(aa) )
            {
                // cerr << "call_0" << endl;
                cref->f( aa );
                //                cerr << "call_1" << endl;
            }
//            else
//            {
//                cerr << "encore rate." << endl;
//            }
        };

        cref->async_ref = new UVAsync( &_loop, this, cref->fc );
        cref->sub_topic = channel_name + "";
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
                // cerr << "queue_size=" << cref->queue.size() << endl;
                // this->publish( cref->sub_topic, (void*)(0) );
                cref->async_ref->trigger();
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

private:
    // Job Scheduler
//    std::shared_ptr< JobScheduler > _scheduler;
    cds::job::Context * cds_ctx;
    std::vector< std::thread > _workers;
    void workerRoutine(cds::job::Context *jobCtx, cds::job::Job *rootJob);
    int _numAdditional;
    cds::job::Job* _rootJob;
    std::mutex mtx;

    // UV - experimental
    uv_loop_t _loop;
    cds::job::Job *createUVLoopJob(cds::job::Job *parent = 0);
    cds::job::Job *createRootJob();

    // UV - timers
    std::vector< UVTimer* > _uvtimers;

    // UV - io poll
    std::vector< UVPoller* > _uvpollers;

    // Timer
    double _minTimerInterval;

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
