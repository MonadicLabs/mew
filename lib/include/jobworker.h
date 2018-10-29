#ifndef JOBWORKER_H
#define JOBWORKER_H
#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <mutex>

#include <unistd.h>

#include "mewconfig.h"
#include "workstealingqueue.h"
#include "fifoqueue.h"
#include "job.h"
#include "utils.h"

#ifdef MEW_USE_PROFILING
#include "Remotery.h"
#endif

namespace mew
{

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
            j->_scheduleTime = hrtime();
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
            // _singleThreadQueue = new FIFOQueue< Job* >();
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
        {
            j->_scheduleTime = hrtime();
            return _singleThreadQueue->push(j);
        }
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

}

#endif
