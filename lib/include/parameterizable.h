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

        bool declare_parameter(const std::string &paramName, Value::Type type, Value defaultValue);
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
