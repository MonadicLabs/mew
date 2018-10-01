#pragma once

#include <cstdint>

namespace mew
{
    template<typename T>
    class Identifiable
    {
    public:
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

    private:

    protected:
        T _id;

    };

    typedef Identifiable<uint16_t> ShortIdentifiable;

}
