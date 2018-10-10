#pragma once

#include <memory>

#include <json11.hpp>
using namespace json11;

namespace mew
{
    class Message
    {
    public:
        Message()
            :_versionMajor(1), _versionMinor(0)
        {}

        virtual ~Message(){}

        static std::shared_ptr< Message > fromPayload( const std::string& payload );
        static std::shared_ptr< Message > craftErrorMessage(int errorCode, const std::string& description );

        std::string& type()
        {
            return _type;
        }

        uint16_t& version_minor()
        {
            return _versionMinor;
        }

        uint16_t& version_major()
        {
            return _versionMajor;
        }

        std::string toString();

        bool setPayload( const std::string jsonPayload )
        {
            return false;
        }

        bool setPayload( json11::Json::object payload )
        {
            _payload = payload;
            return true;
        }

        Json::object getPayload()
        {
            return _payload;
        }

    private:
        std::string _type;
        uint16_t _versionMajor;
        uint16_t _versionMinor;

        json11::Json::object _payload;
        std::string _payloadStr;

    protected:

    };
}
