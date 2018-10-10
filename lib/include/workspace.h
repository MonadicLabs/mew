#pragma once

#include <set>
#include <map>

#include "mew.h"
#include "node.h"
#include "identifiable.h"

namespace mew {
class WorkSpace
{
    friend class Node;

public:
    WorkSpace()
    {
        init();
    }

    virtual ~WorkSpace()
    {
        destroy();
    }

    mew::Graph* createEmptyGraph();

    // Entity management
    EntityIdentifiable::ValueType assignId( EntityIdentifiable* entity );
    EntityIdentifiable::ValueType registerId( EntityIdentifiable::ValueType id );

    mew::Mew* getRuntime()
    {
        return _runtime;
    }

    void run()
    {
        return _runtime->run();
    }

    std::vector< mew::Graph* > listGraphs()
    {
        std::vector< mew::Graph* > ret;
        for( auto& kv : _graphs )
        {
            ret.push_back( kv.second );
        }
        return ret;
    }

    Graph* getGraph( const std::string& str_id );
    Graph* getGraph( uint32_t id );

private:
    //
    void init();
    void destroy();

protected:
    // Entity management
    std::set< EntityIdentifiable::ValueType > _idDictionary;
    std::map< EntityIdentifiable::ValueType, mew::Graph* > _graphs;

    // Runtime
    mew::Mew * _runtime;

protected:

};
}
