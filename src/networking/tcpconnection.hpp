#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <queue>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

class TCPConnection : std::enable_shared_from_this<TCPConnection>
{
public:

    TCPConnection(tcp::socket socket)
    : socket_{std::move(socket)},
      queue_{},
      timer_{socket_.get_executor()}
    {
    }

    /** Starts the writer waiting for new messages to be send to the queue. */
    asio::awaitable<void> writer();

    /** Sends the given message to the connected client. */
    asio::awaitable<void> send(std::string_view message, bool async);

    /** Reads a message from the connected client. */
    asio::awaitable<std::string> read(std::string& read_buffer);

    tcp::socket& getSocket() { return socket_; }

private:

    tcp::socket socket_;
    std::queue<std::string> queue_;
    asio::steady_timer timer_;
};

#endif