#pragma once

#include <cstdint>
#include <string>

#include "proquint.h"

namespace mew
{
    template<typename T>
    class Identifiable
    {
    public:
        typedef T ValueType;
        Identifiable( const T& id_value = 0 )
        {

        }

        virtual ~Identifiable()
        {

        }

        T id()
        {
            return _id;
        }

        void set_id( T id_value )
        {
            _id = id_value;
        }

        std::string str_id()
        {
            char tmpBuffer[1024];
            uint2quint( tmpBuffer, id(), '-' );
            tmpBuffer[11] = '\0';
            std::string ret = tmpBuffer;
            return ret;
        }

    private:

    protected:
        T _id;

    };

    typedef Identifiable<uint16_t> ShortIdentifiable;
    typedef Identifiable<uint32_t> LongIdentifiable;

    //
    typedef LongIdentifiable EntityIdentifiable;

}
