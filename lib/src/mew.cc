
#include "mew.h"
#include "uvtimer.h"

#define CDS_JOB_IMPLEMENTATION
#include "cds_job.h"

mew::Mew::Mew()
{
    // UV - experimental
    uv_loop_init( &_loop );

    _minTimerInterval = 0.1;
    _numAdditional = std::thread::hardware_concurrency() - 1;
    // cerr << "Number of additionnal threads = " << _numAdditional << endl;
    _numAdditional = 0;

    // cds_job
    cds_ctx = cds::job::createContext(_numAdditional + 1, 32);
    cds::job::initWorker(cds_ctx);

    if( _numAdditional == 0 )
    {
        _minTimerInterval = 0.001;
    }

    _rootJob = createUVLoopJob();
    cds::job::enqueueJob( _rootJob );

}

void mew::Mew::run()
{

    /*
    // Start workers
    for(int iThread=0; iThread<_numAdditional; iThread+=1) {
        // workers[iThread] = std::thread(workerTest, cdsx, rootJob);
        _workers.push_back( std::thread(&Mew::workerRoutine, this, cds_ctx, _rootJob) );
    }
    */

    // lol wait ?
    waitForJob(_rootJob);

}

bool mew::Mew::unsubscribe(void *subReference)
{
    std::unique_lock< std::mutex >( _subRegistryMtx );
    bool removed = false;
    for( auto& kv : _subscriptions )
    {
        int idx = -1;
        for( int k = 0; k < kv.second.size(); ++k )
        {
            if( kv.second[ k ] == subReference )
            {
                idx = k;
                removed = true;
                break;
            }
        }
        if( idx >= 0 )
        {
            cerr << "idx=" << idx << endl;
            cerr << "key=" << kv.first << " P_SUBS=" << kv.second.size() << endl;
            SubscriptionReference* csubref = (SubscriptionReference*)subReference;
            kv.second.erase( kv.second.begin() + idx );
            // Wait for jobs completion
            while( csubref->jobCpt != 0 )
            {
                usleep(1000);
            }
            delete csubref;
            //
            cerr << "key=" << kv.first << " P_SUBS=" << kv.second.size() << endl;
        }
        if( removed )
            break;
    }
    cerr << "SUBSCRIPTIONS=" << _subscriptions.size() << endl;
    return removed;
}

void mew::Mew::printSubscriptions()
{
    std::unique_lock< std::mutex >( _subRegistryMtx );
    for( auto& kv : _subscriptions )
    {
        cerr << "topic: " << kv.first << endl;
        for( int k = 0; k < kv.second.size(); ++k )
        {
            cerr << "\t\t sref=" << kv.second[ k ] << endl;
        }
        cerr << endl;
    }
}

void mew::Mew::set_timer_interval(void *timer_ref, double dt_secs)
{

}

void mew::Mew::workerRoutine( cds::job::Context* jobCtx, cds::job::Job* rootJob )
{
    cds::job::initWorker(jobCtx);
    waitForJob(rootJob);
}

cds::job::Job* mew::Mew::createUVLoopJob( cds::job::Job* parent )
{
    /*
    if( _numAdditional > 0 )
    {
        Job * uvJob = new Job( []( Job* j ){
                mew::Mew * m = (mew::Mew*)j->userData();
                uv_run( &(m->_loop), UV_RUN_ONCE);
                Job * nextJob = m->createUVLoopJob();
                m->scheduler()->push( nextJob );
        }, this );
        uvJob->label() = "UV_UPDATE";
        return uvJob;
    }
    else
    {
        Job * uvJob = new Job( []( Job* j ){
                mew::Mew * m = (mew::Mew*)j->userData();
                uv_run( &(m->_loop), UV_RUN_ONCE);
                Job * nextJob = m->createUVLoopJob();
                m->scheduler()->push( nextJob );
        }, this );
        uvJob->label() = "UV_UPDATE";
        return uvJob;
    }
    */

    if( parent != 0 )
    {
        //        cds::job::waitForJob( parent );
    }

    cds::job::Job * jj = cds::job::createJob([](cds::job::Job* j, const void* userdata){
            mew::Mew* self = (mew::Mew*)userdata;
            // self->mtx.lock();
            cerr << "coucou " << j << endl;
            int runret = uv_run( &(self->_loop), UV_RUN_NOWAIT);
            cerr << "runret=" << runret << endl;
            sleep(1);
            cds::job::Job* nextJob = self->createUVLoopJob(j);
            int eret = cds::job::enqueueJob( self->createUVLoopJob(j) );
            cerr << "eret=" << eret << endl;
//            cds::job::waitForJob( nextJob );
            // self->mtx.unlock();
    }, parent, this, sizeof(mew::Mew*) );
    cerr << "jj=" << jj << endl;
    return jj;

}

cds::job::Job *mew::Mew::createRootJob()
{
    /*
    cds::job::Job * jj = cds::job::createJob([](cds::job::Job* j, const void* userdata){
            mew::Mew* self = (mew::Mew*)userdata;
            // self->mtx.lock();
            cerr << "ROOT coucou " << j << endl;
            int runret = uv_run( &(self->_loop), UV_RUN_ONCE);
             cerr << "runret=" << runret << endl;
            sleep(1);
            cds::job::enqueueJob( self->createUVLoopJob(j) );
            // self->mtx.unlock();
    }, parent, this, sizeof(mew::Mew*) );
    cerr << "jj=" << jj << endl;
    return jj;
    */
    return nullptr;
}

void mew::Mew::processSubscriber(mew::Mew::SubscriptionReference *sref)
{
    cpp::any aobj;
    if( sref->queue.try_dequeue( aobj ) )
    {
        cerr << "." << endl;
        sref->f( aobj );
    }
}

mew::Mew *mew::mew_init(int argc, char **argv)
{
    return new mew::Mew();
}

#include <cxxabi.h>
const char *demangle(const char *name)
{
    static char buf[1024];
    size_t size = sizeof(buf);
    int status;
    // todo:
    char* res = abi::__cxa_demangle (name,
                                     buf,
                                     &size,
                                     &status);
    buf[sizeof(buf) - 1] = 0; // I'd hope __cxa_demangle does this when the name is huge, but just in case.
    return res;
}
