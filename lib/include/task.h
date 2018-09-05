#pragma once

#include <functional>

#include "any.h"

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
        TIMER
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
    TimerTask( std::function<void(double)> f )
        :Task(TIMER), _f(f)
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

private:

};

}
