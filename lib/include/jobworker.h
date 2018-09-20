#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>

#include "mewconfig.h"
#include "wsdeque.h"
#include "job.h"

#ifdef MEW_USE_PROFILING
#include "Remotery.h"
#endif

namespace mew
{
class JobWorker;
class Job : public std::enable_shared_from_this<Job>
{

    friend class JobWorker;

public:

    Job()
        :_assignedWorker(nullptr), _f(nullptr)
    {

    }

    template<typename F>
    Job( F&& f = nullptr, void * userData = nullptr )
        :_assignedWorker(nullptr), _userData(userData)
    {
        _f = [&f, this]{
            f( shared_from_this() );
        };
    }

    virtual ~Job()
    {

    }

    template<typename F>
    void setFunction( F&& f = nullptr )
    {
        _f = [=]{
            f( this );
        };
    }

    void pushChild(std::shared_ptr<Job> j );

    void run()
    {
#ifdef MEW_USE_PROFILING
        rmt_BeginCPUSample(JOB_RUN, 0);
#endif
        // test();
        _f();
#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
    }

    void test()
    {
        std::cerr << "job_test." << std::endl;
    }

    void * userData()
    {
        return _userData;
    }

private:
    std::function<void(void)> _f;
    JobWorker* _assignedWorker;
    void * _userData;

protected:

};

class JobScheduler;
class JobWorker
{
public:
    JobWorker( JobScheduler* parentScheduler = nullptr )
        :_parentScheduler(parentScheduler),
          _queue(std::make_shared< deque::Deque< std::shared_ptr<Job> > >()),
          _wep(_queue), _sep(_queue)
    {
        //        _queue = std::make_shared< wsdeque::Deque< std::shared_ptr<Job> > >();
    }

    virtual ~JobWorker()
    {

    }

    void run();

    void push( std::shared_ptr<Job> j )
    {
        _wep.push( j );
    }

    bool pop( std::shared_ptr<Job>& j )
    {
        experimental::optional< std::shared_ptr<Job> > op = _wep.pop();
        if( op )
        {
            j = *op;
            return true;
        }
        else
        {
            j = nullptr;
            return false;
        }
    }

    bool steal( std::shared_ptr<Job>& j )
    {
        //        cerr << "stealing." << endl;
        auto sep = _sep;
        experimental::optional< std::shared_ptr<Job> > op = sep.steal();
        if( op )
        {
//                        cerr << "steal ok" << endl;
            j = *op;
            return true;
        }
        else
        {
            //            cerr << "steal nok" << endl;
            j = nullptr;
            return false;
        }
    }

private:
    JobScheduler * _parentScheduler;
    std::shared_ptr<deque::Deque< std::shared_ptr<Job> > > _queue;
    deque::Worker< std::shared_ptr<Job> > _wep;
    deque::Stealer< std::shared_ptr<Job> > _sep;

protected:

};

class JobScheduler : public JobWorker
{
public:
    JobScheduler()
        :JobWorker(this)
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

        _workers.push_back( this );
        spawnAdditionalWorkers();
    }

    bool trySteal( std::shared_ptr<Job>& j, JobWorker* caller )
    {
//        cerr << "trySteal... ";
        bool ret = false;
        // select a worker randomly
        size_t k = rand() % _workers.size();
        if( _workers[k] != caller )
        {
            ret = _workers[k]->steal( j );
        }
        else
        {
            j = nullptr;
            ret = false;
        }
//        if( ret )
//            cerr << "OK" << endl;
//        else
//            cerr << "NOK" << endl;
        return ret;
    }

private:
    void spawnAdditionalWorkers()
    {
        unsigned concurentThreadsSupported = 8; //std::thread::hardware_concurrency();
        for( unsigned int k = 0; k < concurentThreadsSupported - 1; ++k )
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

protected:

};

void Job::pushChild(std::shared_ptr<Job> j)
{
    if( _assignedWorker )
    {
        _assignedWorker->push( j );
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
        std::shared_ptr<Job> j = nullptr;
        if( pop(j) )
        {
            //            cerr << "pop ok j = " << j << endl;
            j->_assignedWorker = this;
            //            cerr << "will run" << endl;
            j->run();
            //            cerr << "has run" << endl;
            j->_assignedWorker = nullptr;
            //            delete j;
            continue;
        }
        else
        {
            // try to steal
            if( _parentScheduler )
            {
                //                cerr << "parent=" << _parentScheduler << endl;
                if( _parentScheduler->trySteal( j, this ) )
                {
                    //                    cerr << "stole a job !" << endl;
                    j->_assignedWorker = this;
                    j->run();
                    j->_assignedWorker = nullptr;
                    std::this_thread::sleep_for( std::chrono::microseconds(10) );
                }
                else
                {
                    // could not steal
                    //                  cerr << "could not steal !" << endl;
                    std::this_thread::sleep_for( std::chrono::microseconds(10) );
                }
            }
        }
        std::this_thread::sleep_for( std::chrono::microseconds(10) );
    }
}

}

#ifdef MEW_USE_PROFILING
#include "Remotery.c"
#endif

