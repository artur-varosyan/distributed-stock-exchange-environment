#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/optional.hpp>

#include "networkentity.hpp"
#include "../agent/agent.hpp"
#include "../agent/agentfactory.hpp"
#include "../message/message.hpp"
#include "../message/messagetype.hpp"
#include "../message/market_data_message.hpp"
#include "../message/exec_report_message.hpp"
#include "../message/subscribe_message.hpp"
#include "../message/limit_order_message.hpp"
#include "../message/market_order_message.hpp"
#include "../message/cancel_order_message.hpp"
#include "../message/event_message.hpp"
#include "../message/cancel_reject_message.hpp"
#include "../message/config_message.hpp"
#include "../message/config_ack_message.hpp"
#include "../order/order.hpp"
#include "../order/limitorder.hpp"
#include "../order/marketorder.hpp"
#include "../trade/trade.hpp"

BOOST_CLASS_EXPORT(Message);
BOOST_CLASS_EXPORT(MarketDataMessage);
BOOST_CLASS_EXPORT(SubscribeMessage);
BOOST_CLASS_EXPORT(LimitOrderMessage);
BOOST_CLASS_EXPORT(MarketOrderMessage);
BOOST_CLASS_EXPORT(CancelOrderMessage);
BOOST_CLASS_EXPORT(EventMessage);
BOOST_CLASS_EXPORT(CancelRejectMessage);

/** TODO: This should be elsewhere */
BOOST_CLASS_EXPORT(AgentConfig);
BOOST_CLASS_EXPORT(ExchangeConfig);
BOOST_CLASS_EXPORT(TraderConfig);
BOOST_CLASS_EXPORT(ArbitrageurConfig);
BOOST_CLASS_EXPORT(MarketWatcherConfig);
BOOST_CLASS_EXPORT(ZIPConfig);

BOOST_CLASS_EXPORT(ConfigMessage);
BOOST_CLASS_EXPORT(ConfigAckMessage);

BOOST_CLASS_EXPORT(LimitOrder);
BOOST_CLASS_EXPORT(MarketOrder);
BOOST_CLASS_EXPORT(Order);
BOOST_CLASS_EXPORT(CSVPrintable);
BOOST_CLASS_EXPORT(Trade);
BOOST_CLASS_EXPORT(ExecutionReportMessage);

namespace archive = boost::archive;

void NetworkEntity::start()
{
    asio::co_spawn(io_context_, TCPServer::start(), asio::detached);
    asio::co_spawn(io_context_, UDPServer::start(), asio::detached);
    std::cout << "Listening on port " << port() << "...\n";

    io_context_.run();
}

std::string NetworkEntity::serialiseMessage(MessagePtr message)
{
    std::stringstream ss;
    archive::text_oarchive oa{ss};
    oa << message;
    return ss.str() + std::string{"#END#"};
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
    message->markSent(agent()->getAgentId());
    asio::post(io_context_, [=, this](){
        asio::co_spawn(this->io_context_, UDPServer::sendBroadcast(pair.first, pair.second, serialiseMessage(message)), asio::detached);
    });
}

void NetworkEntity::sendMessage(ipv4_view address, MessagePtr message, bool async)
{
    // If TCP connection exists with the given address, send the message
    if (connections_.left.find(std::string{address}) != connections_.left.end())
    {
        TCPConnectionPtr connection = connections_.left.at(std::string{address});
        message->markSent(agent()->getAgentId());
        asio::post(io_context_, [=, this](){
            asio::co_spawn(io_context_, TCPServer::sendMessage(connection, serialiseMessage(message), async), asio::detached);
        });
        
    }
    // Abort sending message if TCP connection cannot be found
    else 
    {
        std::cout << "Message failed to send: no TCP connection with " << address << "\n";
    }

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
    std::string full_addr = concatAddress(address, port);
    if (connections_.left.find(full_addr) != connections_.left.end())
    {
        connections_.left.erase(full_addr);
    }
}


std::string NetworkEntity::handleMessage(std::string_view sender_adress, unsigned int sender_port, std::string_view message)
{
    // std::cout << "Received message from " << sender_adress << ":" << sender_port << "\n";
    // std::cout << "Received message " << message << "\n";

    try     
    {
        MessagePtr msg = deserialiseMessage(message);
        msg->markReceived();

        if (msg->type == MessageType::CONFIG)
        {
            configureEntity(sender_adress, std::dynamic_pointer_cast<ConfigMessage>(msg));
        }
        else
        {
            std::optional<MessagePtr> response = agent()->handleMessage(concatAddress(sender_adress, sender_port), msg);
            if (response.has_value()) 
            {
                return serialiseMessage(response.value());
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message from " << sender_adress << "\n";
        std::cout << "Reason: " << e.what() << "\n";
        std::cout << "Message " << message << "\n";
    }

    return std::string{};
}

void NetworkEntity::handleBroadcast(std::string_view sender_adress, unsigned int sender_port, std::string_view message)
{
    // std::cout << "Received broadcast from " << sender_adress << ":" << sender_port << ": " << message << "\n";

    try 
    {
        MessagePtr msg = deserialiseMessage(message);
        msg->markReceived();
        agent()->handleBroadcast(concatAddress(sender_adress, sender_port), msg);
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to deserialise message from " << sender_adress << "\n";
        std::cout << e.what() << "\n";
    }
    
    return;
}

unsigned int NetworkEntity::port()
{
    return port_;
}

std::string NetworkEntity::addr()
{
    if (!addr_.has_value())
    {
        throw std::runtime_error("The address of the network entity is unknown.");
    }

    return addr_.value();
}

void NetworkEntity::setAgent(std::shared_ptr<Agent> agent)
{
    if (agent_.has_value())
    {
        agent_.value()->terminate();
    }
    
    agent_ = agent;
    agent->start();
}

std::shared_ptr<Agent> NetworkEntity::agent()
{
    if (!agent_.has_value())
    {
        throw std::runtime_error("No agent has been assigned to this NetworkEntity.");
    }

    return agent_.value();
}

void NetworkEntity::configureEntity(std::string_view sender_address, ConfigMessagePtr msg)
{
    // Set own address
    addr_ = splitAddress(msg->config->addr).first;

    // Initialise a new agent
    setAgent(AgentFactory::createAgent(this, msg->config));

    // Send configuration acknowledgement back to orchestrator
    // ConfigAckMessagePtr ack_msg = std::make_shared<ConfigAckMessage>();
    // sendMessage(sender_address, std::static_pointer_cast<Message>(ack_msg), true);
}