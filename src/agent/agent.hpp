#ifndef AGENT_HPP
#define AGENT_HPP

#include <iostream>
#include <boost/asio.hpp>

#include "../networking/networkentity.hpp"
#include "../message/message.hpp"
#include "../message/messagetype.hpp"

namespace asio = boost::asio;

class Agent : private NetworkEntity
{
public:

    Agent(asio::io_context& io_context)
    : io_context_(io_context),
      NetworkEntity(io_context, kTcpPort, kUdpPort)
    {
    }
    
    virtual ~Agent() = default;

    void start()
    {
        NetworkEntity::start();
    }

    /** Handles an incoming message and returns the message to send. All derived classes must implement this. */
    virtual std::optional<Message> handleMessage(Message message) override = 0;

    /** Handles an incoming broadcast. All derived classes must implement this. */
    virtual void handleBroadcast(Message message) override = 0;

private:

    static constexpr unsigned int kTcpPort = 8080;
    static constexpr unsigned int kUdpPort = 8080;

    asio::io_context& io_context_;
};

#endif