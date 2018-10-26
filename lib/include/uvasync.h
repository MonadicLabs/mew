#pragma once

#include <memory>
#include <functional>

#include <uv.h>

#include <iostream>
using namespace std;

namespace mew
{

class Mew;
class UVAsync
{
public:
    UVAsync( uv_loop_t* loop,
             mew::Mew* context,
             std::function<void(mew::Mew*)> f )
        : m_async(std::make_shared<uv_async_t>()),
          _context( context )
    {
        _f = f;

        //
        m_async->data = this; // Note: m_timer->data is a void*

        // typedef void (*uv_poll_cb)(uv_poll_t* handle, int status, int events);
        // Start it.
        int uvaret = uv_async_init(loop, m_async.get(), [](uv_async_t* a)
        {
                UVAsync * self = (UVAsync*)a->data;
                self->periodic();
        });

        cerr << "uvaret=" << uvaret << endl;

    }

    virtual ~UVAsync()
    {

    }

    void trigger()
    {
        uv_async_send( m_async.get() );
    }

    void periodic();

    void exec();

private:
    std::shared_ptr<uv_async_t> m_async;
    std::function<void(mew::Mew*)> _f;
    mew::Mew * _context;

protected:

};
}
