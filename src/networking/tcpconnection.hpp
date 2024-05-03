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

    /** Queues the given message to be sent to the connected client. */
    asio::awaitable<void> send(std::string_view message, bool async);

    /** Reads a message from the connected client. */
    asio::awaitable<std::string> read(std::string& read_buffer);

    /** Closes the connection. */
    void close();

    /** Indicates whether the connection is open. */
    bool open();

    /** Returns the outgoing message queue for this connection. */
    std::queue<std::string>& queue() { return queue_; };

    /** Returns the socket associated with this connection. */
    tcp::socket& socket() { return socket_; };

    /** Returns the timer associated with this connection. */
    asio::steady_timer& timer() { return timer_; };

private:

    tcp::socket socket_;
    std::queue<std::string> queue_;
    asio::steady_timer timer_;
};

#endif