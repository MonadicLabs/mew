#pragma once

#include <memory>
#include <functional>

#include <uv.h>

#include <iostream>
using namespace std;

namespace mew
{

class Mew;
class UVPoller
{
public:
    UVPoller( uv_loop_t* loop,
             mew::Mew* context,
             std::function<void(mew::Mew*, int filedescriptor)> f,
             int filedescritptor )
        : m_poll(std::make_shared<uv_poll_t>()),
          _context( context ),
          _fd(filedescritptor)
    {
        _f = f;

        // Initialize the timer.
        uv_poll_init(loop, m_poll.get(), _fd);

        //
        m_poll->data = this; // Note: m_timer->data is a void*

        // typedef void (*uv_poll_cb)(uv_poll_t* handle, int status, int events);
        // Start it.
        int uvpret = uv_poll_start(m_poll.get(), UV_READABLE, [](uv_poll_t* p, int status, int events)
        {
                UVPoller * self = (UVPoller*)p->data;
                self->periodic();
        });

        cerr << "uvpret=" << uvpret << endl;

    }

    virtual ~UVPoller()
    {

    }

    void
    periodic();

    void exec();

private:
    std::shared_ptr<uv_poll_t> m_poll;
    std::function<void(mew::Mew*, int fd)> _f;
    mew::Mew * _context;
    int _fd;

protected:

};
}
