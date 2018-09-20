#pragma once

#include <functional>

#include "cppbackports/any.h"

namespace mew
{

class DataTask;
class Task
{
public:
    typedef enum
    {
        NONE=0,
        DATA,
        TIMER,
        IO,
        IO_POLL
    } Type;

    Task( Type type = NONE )
        :_type(type)
    {

    }

    virtual ~Task()
    {

    }

    Type type()
    {
        return _type;
    }

private:
    Type _type;

};

class DataTask : public Task
{
public:
    DataTask( std::function<void(cpp::any)> f, cpp::any data )
        :Task(DATA), _f(f), _data(data)
    {

    }

    virtual ~DataTask()
    {

    }

    std::function<void(cpp::any)> _f;
    cpp::any _data;

private:

};

class TimerTask : public Task
{
public:
    TimerTask( std::function<void(double)> f, double triggerTime )
        :Task(TIMER), _f(f), _triggerTime(triggerTime)
    {

    }

    TimerTask( const TimerTask& other )
    {
        *this = other;
    }

    TimerTask& operator = (const TimerTask& other)
    {
        if( &other == this )
            return *this;
        _f = other._f;
        return *this;
    }

    virtual ~TimerTask()
    {

    }

    std::function<void(double)> _f;
    double _triggerTime;

private:

};

class IOTask : public Task
{
public:
    IOTask( std::function<void(int)> f, int fd )
        :Task(IO), _f(f), _fd(fd)
    {

    }

    virtual ~IOTask()
    {

    }

    std::function<void(int)> _f;
    int _fd;

private:

protected:

};

class IOPollTask : public Task
{
public:
    IOPollTask( double timeout_sec )
        :Task(IO_POLL), _timeout_sec(timeout_sec)
    {

    }

    virtual ~IOPollTask()
    {

    }

    double _timeout_sec;

};

}
