
#include "identifiable.h"

#include <proquint.h>

std::string mew::EntityIdentifiable::str_id()
{
    char tmpBuffer[1024];
    uint2quint( tmpBuffer, _id, '-' );
    tmpBuffer[11] = '\0';
    std::string ret = std::string( tmpBuffer );
    return ret;
}
