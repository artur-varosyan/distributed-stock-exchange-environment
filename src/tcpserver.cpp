#include "tcpserver.hpp"
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

void TCPServer::start()
{
    {
        std::cout << "Start called" << "\n";

        asio::signal_set signals(io_context_, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ io_context_.stop(); });

        asio::co_spawn(io_context_, listener(), asio::detached);
        io_context_.run();
    }
}

asio::awaitable<void> TCPServer::handleAccept(tcp::socket socket)
{
    std::cout << "Accepted connection from " << socket.remote_endpoint() << "\n";

    try
    {
        char data[1024];
        while (true)
        {
            /** TODO: Ensure that the server can send messages while also waiting for incoming messages */

            std::size_t n = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
            std::cout << "Message from " << socket.remote_endpoint() << ": " << std::string(data, n) << "\n";

            co_await asio::async_write(socket, asio::buffer(data, n), asio::use_awaitable);
        }
    }
    catch (std::exception& e)
    {
        std::printf("handleAccept Exception: %s\n", e.what());
    }
}

asio::awaitable<void> TCPServer::listener()
{
    std::cout << "Listener called" << "\n";

    auto executor = co_await asio::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), port_});
    std::cout << "Listening on port " << port_ << "\n";

    while (true)
    {
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(executor, handleAccept(std::move(socket)), asio::detached);
    }
}