#pragma once

#include "jobscheduler.h"
#include "job.h"
#include "thread_pool.hpp"

namespace mew
{
    class TPScheduler : public JobScheduler
    {
    public:
        TPScheduler()
        {

        }

        virtual ~TPScheduler()
        {

        }

        virtual bool schedule( Job* j )
        {
            return _tp.tryPost( [j](){
                j->run();
                delete j;
            });
        }

    private:
        tp::ThreadPool _tp;

    protected:

    };
}
