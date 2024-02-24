#ifndef NETWORK_ENTITY_HPP
#define NETWORK_ENTITY_HPP

#include <iostream>
#include <boost/asio.hpp>

#include "tcpserver.hpp"
#include "udpserver.hpp"
#include "../message/message.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::ip::udp;

class NetworkEntity : private TCPServer, private UDPServer
{
public:

    NetworkEntity() = delete;
    virtual ~NetworkEntity() = default;

    NetworkEntity(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port)
    : io_context_(io_context),
      TCPServer(io_context, tcp_port),
      UDPServer(io_context, udp_port)
    {
    }

    /** Starts both servers sharing the same context */
    void start()
    {
        asio::co_spawn(io_context_, TCPServer::start(), asio::detached);
        asio::co_spawn(io_context_, UDPServer::start(), asio::detached);
        io_context_.run();
    }

    /** Handles an incoming message and returns the message to send. All derived classes must implement this. */
    virtual Message handleMessage(Message message) = 0;

    /** Handles an incoming broadcast. All derived classes must implement this. */
    virtual void handleBroadcast(Message message) = 0;

private:

    /** Handles an incoming TCP message from one of the existing connections. */
    std::string handleMessage(std::string_view sender_adress, std::string_view message) override;

    /** Handles an incoming UDP broadcast. */
    void handleBroadcast(std::string_view sender_adress, std::string_view message) override;

    /** Serialises a message into a string to be sent. */
    std::string serialiseMessage(Message message);

    /** Deserialises incoming strings into messages. */
    Message deserialiseMessage(std::string_view message);

    asio::io_context& io_context_;
};


#endif