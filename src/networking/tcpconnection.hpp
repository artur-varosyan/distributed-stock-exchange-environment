#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

class TCPConnection : std::enable_shared_from_this<TCPConnection>
{
public:

    TCPConnection(tcp::socket socket)
    : socket_{std::move(socket)}
    {
    }

    /** Sends the given message to the connected client. */
    asio::awaitable<void> send(std::string_view message);

    /** Reads a message from the connected client. */
    asio::awaitable<std::string> read();

    tcp::socket& getSocket() { return socket_; }

private:

    tcp::socket socket_;
};

#endif