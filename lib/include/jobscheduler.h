#pragma once

namespace mew
{
    class Job;
    class JobScheduler
    {
    public:
        JobScheduler()
        {

        }

        virtual ~JobScheduler()
        {

        }

        virtual bool schedule( Job* j )
        {
            return false;
        }

    private:

    protected:

    };
}
