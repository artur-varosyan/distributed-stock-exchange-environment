#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "networkentity.hpp"
#include "../message/message.hpp"
#include "../message/messagetype.hpp"
#include "../message/market_data_message.hpp"
#include "../message/order_ack_message.hpp"
#include "../message/subscribe_message.hpp"
#include "../message/limit_order_message.hpp"
#include "../message/market_order_message.hpp"
#include "../message/cancel_order_message.hpp"

BOOST_CLASS_EXPORT(Message);
BOOST_CLASS_EXPORT(MarketDataMessage);
BOOST_CLASS_EXPORT(OrderAckMessage);
BOOST_CLASS_EXPORT(SubscribeMessage);
BOOST_CLASS_EXPORT(LimitOrderMessage);
BOOST_CLASS_EXPORT(MarketOrderMessage);
BOOST_CLASS_EXPORT(CancelOrderMessage);

namespace archive = boost::archive;

void NetworkEntity::start()
{
    asio::co_spawn(io_context_, TCPServer::start(), asio::detached);
    asio::co_spawn(io_context_, UDPServer::start(), asio::detached);
    io_context_.run();
}

std::string NetworkEntity::serialiseMessage(MessagePtr message)
{
    std::stringstream ss;
    archive::text_oarchive oa{ss};
    oa << message;
    return ss.str();
}

MessagePtr NetworkEntity::deserialiseMessage(std::string_view message)
{
    std::stringstream ss{std::string{message}};
    archive::text_iarchive ia{ss};
    MessagePtr msg = std::make_shared<Message>();
    ia >> msg;

    if (msg == nullptr)
    {
        throw std::runtime_error("Deserialisation returned nullptr");
    }

    return msg;
}

void NetworkEntity::connect(ipv4_view address, std::function<void()> const& callback)
{
    std::pair<std::string, unsigned int> pair = splitAddress(address);

    // Cospawn an asio coroutine to connect to address then call callback
    asio::co_spawn(io_context_, TCPServer::connect(pair.first, pair.second, callback), asio::detached);
}

void NetworkEntity::sendBroadcast(ipv4_view address, MessagePtr message)
{
    std::pair<std::string, unsigned int> pair = splitAddress(address);
    // std::cout << "Cospawning an asio coroutine\n";
    asio::post(io_context_, [=, this](){
        asio::co_spawn(this->io_context_, UDPServer::sendBroadcast(pair.first, pair.second, serialiseMessage(message)), asio::detached);
    });
}

void NetworkEntity::sendMessage(ipv4_view address, MessagePtr message)
{
    std::pair<std::string, unsigned int> pair = splitAddress(address);
    TCPConnectionPtr connection = connections_.left.at(concatAddress(pair.first, pair.second));

    asio::post(io_context_, [=, this](){
        // std::cout << "Posted task is running\n" << "\n";
        asio::co_spawn(io_context_, TCPServer::sendMessage(connection, serialiseMessage(message)), asio::detached);
        // std::cout << "Coroutine spawned\n" << "\n";
    });
    // std::cout << "Task posted\n" << "\n";
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
    // std::cout << "New connection from " << address << ":" << port << "\n";
    connections_.left.insert({concatAddress(address, port), connection});
}

void NetworkEntity::removeConnection(std::string_view address, unsigned int port)
{
    connections_.left.erase(concatAddress(address, port));
    // std::cout << "Removed connection from " << address << ":" << port << "\n";
}


std::string NetworkEntity::handleMessage(std::string_view sender_adress, unsigned int sender_port, std::string_view message)
{
    // std::cout << "Received message from " << sender_adress << ":" << sender_port << ": " << message << "\n";

    try 
    {
        MessagePtr msg = deserialiseMessage(message);
        std::optional<MessagePtr> response = handleMessage(concatAddress(sender_adress, sender_port), msg);
        if (response.has_value()) 
        {
            return serialiseMessage(response.value());
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message" << "\n";
        std::cout << e.what() << "\n";
    }

    return std::string{};
}

void NetworkEntity::handleBroadcast(std::string_view sender_adress, unsigned int sender_port, std::string_view message)
{
    // std::cout << "Received broadcast from " << sender_adress << ":" << sender_port << ": " << message << "\n";

    try 
    {
        MessagePtr msg = deserialiseMessage(message);
        handleMessage(concatAddress(sender_adress, sender_port), msg);
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message" << "\n";
        std::cout << e.what() << "\n";
    }
    
    return;
}