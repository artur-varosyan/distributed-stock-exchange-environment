#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <iostream>
#include <boost/asio.hpp>

#include "tcpconnection.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;

class TCPServer
{
public:
    typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

    TCPServer() = delete;

    TCPServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), 
      tcp_port_{port},
      connections_{}
    {
    }

    /** Starts the server. */
    asio::awaitable<void> start();

    /** Handles a message from a client. */
    virtual std::string handleMessage(std::string_view message) = 0;

    /** TODO: Add public method to send messages to given connection. */

private:

    /** Listens for incoming messages from existing TCP connections. */
    asio::awaitable<void> messageListener(TCPConnectionPtr connection);

    /** Handles new incoming TCP connections */
    asio::awaitable<void> handleAccept(tcp::socket socket);

    /** Listens for new incoming TCP connections */
    asio::awaitable<void> listener();

    const unsigned short tcp_port_;
    asio::io_context& io_context_;

    /** TODO: Consider changing into a map */
    std::vector<TCPConnectionPtr> connections_;
};

#endif