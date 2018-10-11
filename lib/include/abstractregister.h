#pragma once

#include "abstractfactory.h"

#include <unordered_map>
#include <map>
#include <memory>
#include <iostream>
#include <vector>

namespace mew
{
    template<class T>
    class AbstractRegister
    {
    public:
        T* create( const std::string& typeName )
        {
            if( _factories.find(typeName) != _factories.end() )
                return _factories[ typeName ]->create();
            else
                return nullptr;
        }

        void registerFactory( const std::string& typeName, std::shared_ptr< mew::AbstractFactory<T> > f )
        {
            // std::cout << "registering type " << typeName << " addr=" << f << std::endl;
            _factories.insert( make_pair(typeName, f) );
        }

        std::vector<std::string> getTypeList()
        {
            std::vector<std::string> ret;
            for( auto kv : _factories )
            {
                ret.push_back(kv.first);
            }
            return ret;
        }

    private:
        std::map< std::string, std::shared_ptr< mew::AbstractFactory<T> > > _factories;
    };
}
