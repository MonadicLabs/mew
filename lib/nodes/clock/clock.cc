
#include "clock.h"
#include "workspace.h"

mew::Clock::Clock(WorkSpace *ctx)
    :Node(ctx)
{
    _cpt = 0;
    declare_output( "out" );
    declare_parameter( "rate", Value::NUMBER, 0.01 );
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
//    cerr << "coucou" << endl;
    out("out")->write( Value(_cpt++) );
}

void mew::Clock::onSetParameter(const string &paramName, Value value)
{
    if( paramName == "rate" )
    {
        setTickInterval( value.number() );
    }
}



REGISTER_NODE(Clock)
