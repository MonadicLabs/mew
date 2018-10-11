
#include "wsmessage.h"

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <json11.hpp>
using namespace json11;

std::shared_ptr<mew::Message> mew::Message::fromPayload(const std::string &payload)
{
    std::shared_ptr<Message> ret = std::make_shared<mew::Message>();
    string err;
    auto json = Json::parse(payload, err);
    if (!err.empty()) {
        cerr << "error parsing message: " << err << endl;
    }

    if( json[ "type" ].is_string() )
    {
        ret->_type = json[ "type" ].string_value();
    }

    if( json[ "version" ].is_string() )
    {
        ret->_versionMajor = 1;
        ret->_versionMinor = 0;
    }

    if( json[ "payload" ].is_object() )
    {
        ret->_payload = json[ "payload" ].object_items();
    }

    return ret;
}

std::shared_ptr<mew::Message> mew::Message::craftErrorMessage(int errorCode, const string &description)
{
    std::shared_ptr<Message> ret = std::make_shared<mew::Message>();
    ret->type() = "errror";
    ret->_payload = Json::object {
        { "description", description },
        { "code", errorCode }
    };
    cerr << "### error_mesage_dump" << ret->toString() << endl;
    return ret;
}

string mew::Message::toString()
{
    Json::object root;

    // Version
    std::stringstream sstr;
    sstr << _versionMajor << "." << _versionMinor;
    root[ "version" ] = sstr.str();

    // Message type
    root[ "type" ] = _type;

    // Dump payload as payload
    cerr << "payload=" << Json(_payload).dump() << endl;
    root[ "payload" ] = Json( _payload );
    cerr << "payload2=" << root[ "payload" ].dump() << endl;
    Json saveObject = root;
    std::string ret = saveObject.dump();
    return ret;

}
