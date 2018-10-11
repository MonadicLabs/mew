#include "parameterizable.h"

Value mew::Parameterizable::getParameter(const string &paramName)
{
    Value ret;
    if( _parameters.find(paramName) != _parameters.end() )
    {
        // If the parameter exists...
        ret = _parameters[ paramName ];
    }
    else
    {
        // LOG and/or throw ERROR
    }
    return ret;
}

void mew::Parameterizable::setParameter(const string &paramName, Value value, bool triggerEvent)
{
    if( _parameters.find(paramName) != _parameters.end() )
    {
        _parameters[ paramName ] = value;
        if( triggerEvent )
            onSetParameter( paramName, value );
    }
}
