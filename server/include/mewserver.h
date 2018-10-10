#pragma once

#include "wsserver.h"
#include "workspace.h"

#include <map>
#include <vector>

namespace mew
{
    class Server
    {
    public:
        Server()
            :_workspace(nullptr)
        {

        }

        virtual ~Server()
        {

        }

        void createNewWorkspace()
        {
            _workspace = new WorkSpace();
        }

        WorkSpace * workspace()
        {
            return _workspace;
        }

    private:
        WorkSpace* _workspace;

    protected:

    };
}
