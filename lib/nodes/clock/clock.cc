
#include "clock.h"
#include "workspace.h"

mew::Clock::Clock(WorkSpace *ctx)
    :Node(ctx)
{
    declare_output( "out" );
    declare_parameter( "rate", Value::NUMBER, 1.0 );
}

mew::Clock::~Clock()
{

}

void mew::Clock::onContextChange(mew::WorkSpace *ctx)
{
    setTickInterval( getParameter("rate").number() );
}

void mew::Clock::onTimer(double dt)
{
    out("out")->write( Value(dt) );
}

void mew::Clock::onSetParameter(const string &paramName, Value value)
{
    if( paramName == "rate" )
    {
        setTickInterval( value.number() );
    }
}



REGISTER_NODE(Clock)
