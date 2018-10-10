
#include <random>
#include <iostream>
using namespace std;

#include "workspace.h"
#include "graph.h"
#include "identifiable.h"
#include "log.h"
#include "proquint.h"

mew::Graph *mew::WorkSpace::createEmptyGraph()
{
    mew::Graph * g = new mew::Graph( this );
    uint32_t gid = assignId( g );
    g->set_id( gid );
    _graphs.insert( make_pair( gid, g ) );
    return g;
}

mew::EntityIdentifiable::ValueType mew::WorkSpace::assignId(mew::EntityIdentifiable *entity)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution< EntityIdentifiable::ValueType > dis( std::numeric_limits<EntityIdentifiable::ValueType>::min(), std::numeric_limits<EntityIdentifiable::ValueType>::max() );
    EntityIdentifiable::ValueType rng_id = dis(gen);

    while( _idDictionary.count( rng_id ) != 0 )
    {
        rng_id = dis(gen);
    }
    entity->set_id( rng_id );

    // Register id
    registerId( rng_id );

    MEW_DEBUG(INFO) << "Assigned id: " << entity->str_id() << " to entity " << entity << endl;

    return rng_id;
}

mew::EntityIdentifiable::ValueType mew::WorkSpace::registerId(mew::EntityIdentifiable::ValueType id)
{
    _idDictionary.insert( 0 );
    return id;
}

mew::Graph *mew::WorkSpace::getGraph(const string &str_id)
{
    uint32_t id = quint2uint( str_id.c_str() );
    return getGraph( id );
}

mew::Graph *mew::WorkSpace::getGraph(uint32_t id)
{
    Graph* ret = nullptr;
    if( _graphs.find( id ) != _graphs.end() )
    {
        ret = _graphs[ id ];
    }
    return ret;
}

void mew::WorkSpace::init()
{
    // Initialize runtime
    _runtime = new mew::Mew();

    //
}

void mew::WorkSpace::destroy()
{

}
