#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <iostream>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

class TCPServer
{
public:

    TCPServer() = delete;

    TCPServer(unsigned short port) 
    : io_context_{}, 
      port_{port},
      sockets_{}
    {
    }

    /** Starts the server. */
    void start();

private:

    /** Handles new incoming TCP connections */
    asio::awaitable<void> handleAccept(tcp::socket socket);

    /** Listens for new incoming TCP connections */
    asio::awaitable<void> listener();

    const unsigned short port_;
    asio::io_context io_context_;

    /** TODO: Add an abstraction for the sockets */
    /** TODO: Consider changing into a map */
    std::vector<tcp::socket> sockets_;
};

#endif