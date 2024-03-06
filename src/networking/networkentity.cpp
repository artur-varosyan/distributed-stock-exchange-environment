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

    try 
    {
        Message msg = deserialiseMessage(message);
        std::optional<Message> response = handleMessage(msg);
        if (response.has_value()) 
        {
            return serialiseMessage(response.value());
        }
        else 
        {
            return std::string{};
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message" << "\n";
    }

    return std::string{};
}

void NetworkEntity::handleBroadcast(std::string_view sender_adress, std::string_view message)
{
    std::cout << "Received broadcast from " << sender_adress << ": " << message << "\n";

    try 
    {
        Message msg = deserialiseMessage(message);
        handleMessage(msg);
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message" << "\n";
    }
    
    return;
}