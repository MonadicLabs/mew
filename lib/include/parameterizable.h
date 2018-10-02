#pragma once

#include <map>
#include <vector>

#include "value.h"

namespace mew
{
    class Parameterizable
    {
    public:
        Parameterizable(){}
        virtual ~Parameterizable(){}

        virtual bool declare_parameter(const std::string &paramName, Value::Type type, Value defaultValue)
        {
            if( _parameters.find(paramName) != _parameters.end() )
            {
                // Parameter already exists
                // LOG_S(WARNING)
                return false;
            }
            else
            {
                Value val( type );
                if( val.type() != defaultValue.type() )
                {
                    // Oops type mismatch
                    return false;
                }
                else
                {
                    val = defaultValue;
                }
                _parameters.insert( make_pair( paramName, val ) );
                return true;
            }
        }
        Value getParameter( const std::string& paramName );
        std::vector< std::string > getParameterList();
        void setParameter( const std::string& paramName, Value value, bool triggerEvent = true );

//        void parseParameters(const std::string &jsonValues);
//        void parseParameters( std::map< std::string, novadem::ParameterValue > vals );

        virtual void onSetParameter( const std::string& paramName, Value value ){}
        void clearParameters();

    private:

    protected:
        std::map< std::string, Value > _parameters;

    };
}
