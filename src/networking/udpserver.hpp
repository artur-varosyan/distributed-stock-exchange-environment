#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include <iostream>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::udp;

class UDPServer
{
public:
    UDPServer() = delete;

    UDPServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), 
      udp_port_{port},
      socket_{io_context_, udp::endpoint(udp::v4(), port)}
    {
    }

    /** Starts the server. */
    asio::awaitable<void> start();

    /** Sends a UDP broadcast message to the given address and port destination. */
    asio::awaitable<void> sendBroadcast(std::string_view address, const unsigned int port, std::string_view message);

    /** Sends a UDP broadcast message to the given UDP endpoint. */
    asio::awaitable<void> sendBroadcast(udp::endpoint endpoint, std::string_view message);

    /** Handles an incoming UDP broadcast. */
    virtual void handleBroadcast(std::string_view sender_address, unsigned int sender_port, std::string_view message) = 0;

private:

    /** Listens for incoming UDP broadcasts. */
    asio::awaitable<void> listener();

    const unsigned short udp_port_;
    asio::io_context& io_context_;
    udp::socket socket_;
};

#endif