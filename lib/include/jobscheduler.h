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

        bool push( Job* j )
        {
            return false;
        }

    private:

    protected:

    };
}
