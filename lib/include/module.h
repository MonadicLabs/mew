#pragma once

#include "mew.h"

namespace mew
{
    class Module
    {
    public:
        Module( Mew* context )
            :_ctx(context)
        {

        }

        virtual ~Module()
        {

        }

    private:

    protected:
        Mew* _ctx;

    };
}
