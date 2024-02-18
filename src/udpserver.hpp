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

    /** TODO: Add public method to send broadcasts to UDP endpoints. */

    /** Handles an incoming UDP broadcast. */
    virtual void handleBroadcast(std::string_view message) = 0;

private:

    /** Listens for incoming UDP broadcasts. */
    asio::awaitable<void> listener();

    const unsigned short udp_port_;
    asio::io_context& io_context_;
    udp::socket socket_;
};

#endif