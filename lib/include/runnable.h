#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

namespace mew
{
    class Runnable
    {
    public:

        typedef enum
        {
            STOPPED = 0,
            RUNNING,
            WAITING_STOP
        } State;

        Runnable(){}
        virtual ~Runnable(){}

        void start()
        {
            _thr = std::thread( &Runnable::run, this );
        }

        void stop()
        {
            setState( WAITING_STOP );
        }

        void waitUntilStopped()
        {
            _thr.join();
        }

        void setState( State s )
        {
            std::lock_guard<std::mutex> lock( _mtx );
            _state = s;
        }

        State getState()
        {
            std::lock_guard<std::mutex> lock( _mtx );
            return _state;
        }

        virtual void run()=0;

        bool hasToStop()
        {
            return (getState() == WAITING_STOP);
        }

    private:
        std::thread _thr;
        std::mutex  _mtx;
        std::condition_variable _cnd;
        State _state;

    protected:

    };
}
