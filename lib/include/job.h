#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <atomic>
#include <mutex>
using namespace std;

#ifdef MEW_USE_PROFILING
#include "Remotery.h"
#endif

namespace mew{

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
        // _joblock.lock();
        // _assignedWorker.store( worker );
        //        cerr << "JOB:" << _label << endl;
#ifdef MEW_USE_PROFILING
        rmt_BeginCPUSampleDynamic( _label.c_str(), 0);
#endif

        _f();

#ifdef MEW_USE_PROFILING
        rmt_EndCPUSample();
#endif
        // _assignedWorker.store( 0 );
        // _joblock.unlock();
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

    double _scheduleTime;
    double _execTime;

private:
    std::function<void(void)> _f;
    std::atomic<JobWorker*> _assignedWorker;
    void * _userData;
    std::mutex _joblock;
    std::atomic<int> _scheduleCounter;
    std::string _label;

protected:

};

}
