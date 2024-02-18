#ifndef NETWORK_ENTITY_HPP
#define NETWORK_ENTITY_HPP

#include <iostream>
#include <boost/asio.hpp>

#include "tcpserver.hpp"
#include "udpserver.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;
using asio::ip::udp;

class NetworkEntity : public TCPServer, public UDPServer
{
public:

    NetworkEntity() = delete;

    NetworkEntity(asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port)
    : io_context_(io_context),
      TCPServer(io_context, tcp_port),
      UDPServer(io_context, udp_port)
    {
    }

    /** Starts both servers sharing the same context */
    asio::awaitable<void> start()
    {
        asio::co_spawn(io_context_, TCPServer::start(), asio::detached);
        asio::co_spawn(io_context_, UDPServer::start(), asio::detached);

        co_return;
    }

    std::string handleMessage(std::string_view message) override
    {
        std::cout << "Received message: " << message << "\n";
        return std::string(message);
    }

    void handleBroadcast(std::string_view message) override
    {
        std::cout << "Received broadcast: " << message << "\n";
    }

private:

    asio::io_context& io_context_;
};


#endif