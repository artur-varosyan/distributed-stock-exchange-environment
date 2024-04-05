#include <iostream>
#include <boost/asio.hpp>
#include <boost/system.hpp>

#include "tcpconnection.hpp"

namespace asio = boost::asio;

asio::awaitable<void> TCPConnection::writer()
{
    try {
        while (socket_.is_open())
        {
            // Wait for new message added to queue
            if (queue_.empty())
            {
                /** TODO: Decide how to handle this error code. */
                boost::system::error_code ec;
                co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));
            }
            // Send message to the connection
            else
            {
                co_await asio::async_write(socket_, asio::buffer(queue_.front()), asio::use_awaitable);
                queue_.pop();
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "TCP connection writer failed. Exception " << e.what() << "\n";
    }
}

asio::awaitable<void> TCPConnection::send(std::string_view message, bool async)
{
    try {
        // Push message to queue
        queue_.push(std::string{message});

        // Notify of new message in queue
        timer_.cancel_one();
    }
    catch (std::exception& e)
    {
        std::cout << "Failed to send message. Exception " << e.what() << "\n";
    }

    co_return;
}

asio::awaitable<std::string> TCPConnection::read(std::string& read_buffer)
{
    // Read until the delimiter
    std::size_t n = co_await asio::async_read_until(socket_, asio::dynamic_buffer(read_buffer, 4096), "#END#", asio::use_awaitable);

    // Extract the message from the buffer
    std::string msg = read_buffer.substr(0, n);

    // Consume the message length worth of bytes from the read buffer
    read_buffer.erase(0, n);
    
    co_return msg;
}