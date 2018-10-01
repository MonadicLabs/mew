#pragma once

#include "mew.h"
#include "port.h"
#include "identifiable.h"
#include "parameterizable.h"

#include <iostream>
using namespace std;

namespace mew
{
    class Node : public ShortIdentifiable, public Parameterizable
    {

    public:
        Node( mew::Mew* ctx )
            :_context(ctx), _tickTimerRef(0), _tickInterval(0.0)
        {
            setTickInterval( 0.1 );
        }

        virtual ~Node()
        {

        }

        virtual void tick( double dt )
        {
            cerr << "tick." << endl;
        }

    private:
        mew::Mew * _context;

    protected:
        // Ticking stuff...
        void setTickInterval( double intervalSeconds )
        {
            if( intervalSeconds > 0.0 )
            {
                if( _tickTimerRef == 0 )
                {
                    std::function<void(mew::Mew*,double)> f = [this](mew::Mew*, double dt){
                        this->tick( dt );
                    };
                    _tickTimerRef = _context->timer( f, intervalSeconds );
                }
                else
                {
                    _context->set_timer_interval( _tickTimerRef, intervalSeconds );
                }
            }
        }
        double _tickInterval;
        void * _tickTimerRef;

        // Ports
        std::map< std::string, Port* > _inputPorts;

    };
}
