
#include "job.h"
#include "jobworker.h"

void mew::Job::pushChild(Job* j)
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
