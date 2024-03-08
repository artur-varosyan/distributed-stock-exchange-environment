#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include "networkentity.hpp"
#include "../message/message.hpp"

namespace archive = boost::archive;

void NetworkEntity::start()
{
    asio::co_spawn(io_context_, TCPServer::start(), asio::detached);
    asio::co_spawn(io_context_, UDPServer::start(), asio::detached);
    io_context_.run();
}

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

void NetworkEntity::connect(ipv4_view address)
{
    std::pair<std::string, unsigned int> pair = splitAddress(address);
    asio::co_spawn(io_context_, TCPServer::connect(pair.first, pair.second), asio::detached);
}

void NetworkEntity::sendBroadcast(ipv4_view address, Message message)
{
    std::pair<std::string, unsigned int> pair = splitAddress(address);
    asio::co_spawn(io_context_, UDPServer::sendBroadcast(pair.first, pair.second, serialiseMessage(message)), asio::detached);
}

void NetworkEntity::sendMessage(ipv4_view address, Message message)
{
    std::pair<std::string, unsigned int> pair = splitAddress(address);
    TCPConnectionPtr connection = connections_.left.at(concatAddress(pair.first, pair.second));
    asio::co_spawn(io_context_, TCPServer::sendMessage(connection, serialiseMessage(message)), asio::detached);
}

std::string NetworkEntity::concatAddress(std::string_view address, unsigned int port)
{
    return std::string{address} + ":" + std::to_string(port);
}

std::pair<std::string, unsigned int> NetworkEntity::splitAddress(std::string_view address)
{
    std::string address_str{address};
    size_t colon_pos = address_str.find(':');
    return {address_str.substr(0, colon_pos), std::stoi(address_str.substr(colon_pos + 1))};
}

void NetworkEntity::addConnection(std::string_view address, unsigned int port, TCPConnectionPtr connection)
{
    connections_.left.insert({concatAddress(address, port), connection});
}

void NetworkEntity::removeConnection(std::string_view address, unsigned int port)
{
    connections_.left.erase(concatAddress(address, port));
    std::cout << "Removed connection from " << address << ":" << port << "\n";
}


std::string NetworkEntity::handleMessage(std::string_view sender_adress, unsigned int sender_port, std::string_view message)
{
    std::cout << "Received broadcast from " << sender_adress << ":" << sender_port << ": " << message << "\n";

    try 
    {
        Message msg = deserialiseMessage(message);
        std::optional<Message> response = handleMessage(concatAddress(sender_adress, sender_port), msg);
        if (response.has_value()) 
        {
            return serialiseMessage(response.value());
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message" << "\n";
    }

    return std::string{};
}

void NetworkEntity::handleBroadcast(std::string_view sender_adress, unsigned int sender_port, std::string_view message)
{
    std::cout << "Received broadcast from " << sender_adress << ":" << sender_port << ": " << message << "\n";

    try 
    {
        Message msg = deserialiseMessage(message);
        handleMessage(concatAddress(sender_adress, sender_port), msg);
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message" << "\n";
    }
    
    return;
}