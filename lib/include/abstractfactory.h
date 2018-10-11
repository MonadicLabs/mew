#pragma once

#include <memory>

namespace mew
{
    template<class T>
    class AbstractFactory
    {
    public:
        virtual T* create()=0;
    };
}
