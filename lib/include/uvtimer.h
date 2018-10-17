#pragma once

#include <memory>
#include <functional>

#include <uv.h>

#include <iostream>
using namespace std;

namespace mew
{

class Mew;
class UVTimer
{
public:
    UVTimer( uv_loop_t* loop,
             mew::Mew* context,
             std::function<void(mew::Mew*, double dt)> f,
             double dt_seconds )
        : m_timer(std::make_shared<uv_timer_t>()),
          _context( context ),
          _dt_seconds( dt_seconds )
    {
        _f = f;

        // Initialize the timer.
        uv_timer_init(loop, m_timer.get());

        //
        m_timer->data = this; // Note: m_timer->data is a void*

        // Start it.
        int uvtret = uv_timer_start(m_timer.get(), [](uv_timer_t* timer) {
                UVTimer * self = (UVTimer*)timer->data;
                self->periodic();
    },

                // Repeat once a second.
                _dt_seconds * 1000.0, _dt_seconds * 1000.0 );

        cerr << "uvtret=" << uvtret << endl;

    }

    virtual ~UVTimer()
    {

    }

    void
    periodic();

    void exec();

private:
    std::shared_ptr<uv_timer_t> m_timer;
    std::function<void(mew::Mew*, double dt)> _f;
    mew::Mew * _context;
    double _dt_seconds;

protected:

};
}
