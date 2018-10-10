#pragma once

#include <cstdint>
#include <string>

#include "proquint.h"

namespace mew
{

class EntityIdentifiable
{
public:
    typedef uint32_t ValueType;
    EntityIdentifiable( const uint32_t& id_value = 0 )
    {

    }

    virtual ~EntityIdentifiable()
    {

    }

    uint32_t id()
    {
        return _id;
    }

    void set_id( uint32_t id_value )
    {
        _id = id_value;
    }

    std::string str_id();

private:

protected:
    uint32_t _id;

};

//typedef Identifiable<uint16_t> ShortIdentifiable;
//typedef Identifiable<uint32_t> LongIdentifiable;

//
//typedef LongIdentifiable EntityIdentifiable;

}
