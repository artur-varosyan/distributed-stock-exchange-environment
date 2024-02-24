#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include "networkentity.hpp"
#include "../message/message.hpp"

namespace archive = boost::archive;

std::string NetworkEntity::serialiseMessage(Message message)
{
    std::stringstream ss;
    archive::text_oarchive oa{ss};
    oa << message;
    return ss.str();
}

Message NetworkEntity::deserialiseMessage(std::string_view message)
{
    std::stringstream ss{std::string{message}};
    archive::text_iarchive ia{ss};
    Message msg;
    ia >> msg;
    return msg;
}

std::string NetworkEntity::handleMessage(std::string_view sender_adress, std::string_view message)
{
    std::cout << "Received message from " << sender_adress << ": " << message << "\n";

    Message msg = deserialiseMessage(message);
    Message response = handleMessage(msg);

    return serialiseMessage(response);
}

void NetworkEntity::handleBroadcast(std::string_view sender_adress, std::string_view message)
{
    Message msg = deserialiseMessage(message);
    handleMessage(msg);
    
    return;
}