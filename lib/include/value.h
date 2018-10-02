#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
using namespace std;

#include <cppbackports/any.h>
using namespace cpp17;

class Value
{
public:

    typedef enum
    {
        NONE = 0,
        NUMBER,
        STRING,
        BOOLEAN,
        BINARY,
        OBJECT,
        ARRAY
    } Type;

    Value()
        :_type(NONE)
    {

    }

    Value( Type type )
        :_type(type)
    {

    }

    Value( double value )
        :_type(NUMBER), _numberValue(value)
    {

    }

    Value( int value )
        :_type(NUMBER), _numberValue(value)
    {}

    Value( const char* value )
        :_type(STRING), _stringValue(value)
    {}

    Value( const std::string& value )
        :_type(STRING), _stringValue(value)
    {}

    Value( bool value )
        :_type(BOOLEAN)
    {
        _boolValue = value;
    }

    Value( const Value& other )
    {
        *this = other;
    }

    Value( const std::vector<Value>& value )
        :_type(Value::ARRAY), _arrayValue(value)
    {

    }

    virtual ~Value()
    {

    }

    Value& operator=( const Value& other )
    {
        _type = other._type;
        _stringValue = other._stringValue;
        _numberValue = other._numberValue;
        _boolValue = other._boolValue;
        _arrayValue = other._arrayValue;
        _objectValue = other._objectValue;
        _binaryValue = other._binaryValue;
        return *this;
    }

    Value::Type type()
    {
        return _type;
    }

    bool is(Value::Type t)
    {
        return t == _type;
    }

    std::map< std::string, Value >& object_items()
    {
        return _objectValue;
    }

    std::vector< Value >& array_items()
    {
        return _arrayValue;
    }

    double number()
    {
        return _numberValue;
    }

    std::string string()
    {
        return _stringValue;
    }

    bool boolean()
    {
        return _boolValue;
    }

    std::vector< uint8_t >& binary()
    {
        return _binaryValue;
    }

    // Overloaded cast operators...
    /*
    operator std::string()
    {
        cerr << "cast operator" << endl;
        return _stringValue;
    }
    operator cpp17::any()
    {
        if( _type == Value::STRING )
        {
            return cpp17::any( _stringValue );
        }
        else if( _type == Value::NUMBER )
        {
            return cpp17::any( _numberValue );
        }
    }
    */
    //

    std::string str()
    {
        std::string ret;
        if( _type == NONE )
        {
            ret = "[None]";
        }
        else if( _type == ARRAY )
        {
            stringstream sstr;
            sstr << "[Array size=" << _arrayValue.size() << "]";
            ret = sstr.str();
        }
        else if( _type == OBJECT )
        {
            ret = "[Object]";
        }
        else if( _type == NUMBER )
        {
            std::stringstream sstr;
            sstr << _numberValue;
            ret = sstr.str();
        }
        else if( _type == STRING )
        {
            ret = _stringValue;
        }
        else if( _type == BOOLEAN )
        {
            if( _boolValue )
                ret = "True";
            else
                ret = "False";
        }
        else if( _type == BINARY )
        {
            ret = "[Binary]";
        }

        return ret;
    }

private:
    Type _type;
    std::string _stringValue;
    double _numberValue;
    bool _boolValue;
    std::vector< Value > _arrayValue;
    std::map< std::string, Value > _objectValue;
    std::vector< uint8_t > _binaryValue;

};
