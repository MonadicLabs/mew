#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <mutex>

#include "mewconfig.h"
#include "workstealingqueue.h"
#include "fifoqueue.h"
#include "job.h"

#ifdef MEW_USE_PROFILING
#include "Remotery.h"
#endif

namespace mew
{

class JobWorker;
class Job
{

    friend class JobWorker;

public:

    Job()
        :_assignedWorker(nullptr), _scheduleCounter(0), _f(nullptr), _label("JOB")
    {
        //        cerr << "job_ctor()" << endl;
    }

    template<typename F>
    Job( F&& f = nullptr, void * userData = nullptr )
        :_assignedWorker(nullptr), _scheduleCounter(0), _userData(userData), _label("JOB")
    {
        //        cerr << "job_ctor_func()" << endl;
        _f = [&f, this]{
            f( this );
        };
    }

    virtual ~Job()
    {
//        if( _joblock.try_lock() )
//            _joblock.unlock();
    }

    Job& operator =( const Job& other )
    {
        cerr << "operator =" << endl;
    }

    template<typename F>
    void setFunction( F&& f = nullptr )
    {
        _f = [=]{
            f( this );
        };
    }

    void pushChild(Job* j );

    void run(JobWorker* worker)
    {
        _joblock.lock();
        _assignedWorker.store( worker );
        //        cerr << "JOB:" << _label << endl;
#ifdef MEW_USE_PROFILING
        rmt_BeginCPUSampleDynamic( _label.c_str(), 0);
#endif
        _f();
#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
        _assignedWorker.store( 0 );
        _joblock.unlock();
    }

    void test()
    {
        std::cerr << "job_test." << std::endl;
    }

    int counter()
    {
        return _scheduleCounter.load();
    }

    void * userData()
    {
        return _userData;
    }

    std::string& label()
    {
        return _label;
    }

private:
    std::function<void(void)> _f;
    std::atomic<JobWorker*> _assignedWorker;
    void * _userData;
    std::mutex _joblock;
    std::atomic<int> _scheduleCounter;
    std::string _label;

protected:

};

class JobScheduler;
class JobWorker
{
public:
    JobWorker( JobScheduler* parentScheduler = nullptr )
        :_parentScheduler(parentScheduler)
    {
        _jobCounter.store( 0 );
    }

    virtual ~JobWorker()
    {

    }

    virtual void run();

    virtual bool push( Job* j )
    {
        bool ret = _queue.push( j );
        if( ret )
        {

        }
        return ret;
    }

    virtual bool pop( Job*& j )
    {
        bool ret = _queue.pop( j );
        if( ret )
        {

        }
        return ret & (j != nullptr);
    }

    virtual bool steal( Job*& j )
    {
        return _queue.steal(j);
    }

private:
    JobScheduler * _parentScheduler;
    WorkStealingQueue< Job* > _queue;
    std::atomic<int> _jobCounter;

protected:

};

class MainThreadJobWorker : public JobWorker
{
public:
    MainThreadJobWorker( JobScheduler* parentScheduler = nullptr, int requiredAdditionnalThreads = 0 )
        :JobWorker(parentScheduler), _singleThreadQueue(nullptr)
    {
        if( requiredAdditionnalThreads == 0 )
        {
            _singleThreadQueue = new FIFOQueue< Job* >();
            cerr << "Using single thread mode !" << endl;
        }
        else
        {
            cerr << "Using multi thread mode" << endl;
        }
    }

    virtual ~MainThreadJobWorker()
    {

    }

    virtual bool push( Job* j )
    {
        if( _singleThreadQueue )
            return _singleThreadQueue->push(j);
        return JobWorker::push( j );
    }

    virtual bool pop( Job*& j )
    {
        if( _singleThreadQueue )
            return _singleThreadQueue->pop(j);
        return JobWorker::pop( j );
    }

    virtual bool steal( Job*& j )
    {
        if( _singleThreadQueue )
            return false;
        return JobWorker::steal( j );
    }

    virtual void run()
    {
        return JobWorker::run();
    }

private:
    AbstractQueue< Job* > * _singleThreadQueue;

protected:

};

class JobScheduler : public MainThreadJobWorker
{
public:
    JobScheduler( int requiredAdditionnalThreads = 0 )
        :_requiredAdditionnalThreads( requiredAdditionnalThreads ), MainThreadJobWorker(this, requiredAdditionnalThreads)
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
        sleep(2);
#endif

        _workers.push_back( this );
        spawnAdditionalWorkers( _requiredAdditionnalThreads );

    }

    bool trySteal( Job*& j, JobWorker* caller )
    {
        JobWorker * stealWorker = caller;
        while( stealWorker == caller )
        {
            // select a worker randomly
            size_t k = rand() % _workers.size();
            stealWorker = _workers[k];
        }

        bool ret = stealWorker->steal( j );
        return ret & (j != nullptr);

    }

private:
    void spawnAdditionalWorkers( int n )
    {
        for( unsigned int k = 0; k < n; ++k )
        {
            cerr << "SPAWN." << endl;
            JobWorker * worker = new JobWorker( this );
            _workers.push_back( worker );
            _workerThreads.push_back(std::thread([worker]()
            {
                worker->run();
            }));
        }
    }

    std::vector< JobWorker* > _workers;
    std::vector< std::thread > _workerThreads;
    int _requiredAdditionnalThreads;

protected:

};

void Job::pushChild(Job* j)
{
    JobWorker * w = _assignedWorker.load();
    if( w )
    {
        if( w->push( j ) )
        {

        }
        else
        {
            cerr << "nope." << endl;
            delete j;
        }
    }
    else
    {
        cerr << "w=" << w << endl;
        delete j;
    }
}

void JobWorker::run()
{
#ifdef MEW_USE_PROFILING
    std::stringstream sstr;
    sstr << "mew_worker_" << this;
    rmt_SetCurrentThreadName( sstr.str().c_str() );
#endif
    while(true)
    {
        Job* j = nullptr;
        if( pop(j) )
        {
            j->run( this );
            delete j;
        }
        else
        {
            // try to steal
            if( _parentScheduler )
            {
                //                cerr << "parent=" << _parentScheduler << endl;
                if( _parentScheduler->trySteal( j, this ) )
                {
                    j->run(this);
                    delete j;
                }
                else
                {
                    std::this_thread::sleep_for( std::chrono::microseconds(100) );
                }
            }
            std::this_thread::sleep_for( std::chrono::microseconds(100) );
        }
    }
}

}

#ifdef MEW_USE_PROFILING
#include "Remotery.c"
#endif

