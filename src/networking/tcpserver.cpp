#include "tcpserver.hpp"
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

asio::awaitable<void> TCPServer::start()
{
    asio::signal_set signals(io_context_, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context_.stop(); });

    co_await listener();
}

asio::awaitable<void> TCPServer::messageListener(TCPConnectionPtr connection)
{
    try {
        while (true)
        {
            std::string message = co_await connection->read();

            std::string response = handleMessage(message);
            co_await connection->send(response);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception in message listener: " << e.what() << "\n";

        // Remove connection from list
        std::remove(connections_.begin(), connections_.end(), connection);
    }
}

asio::awaitable<void> TCPServer::handleAccept(tcp::socket socket)
{
    std::cout << "Accepted connection from " << socket.remote_endpoint() << "\n";
    
    // Create a shared pointer to this connection and add to list
    TCPConnectionPtr connection = std::make_shared<TCPConnection>(std::move(socket));
    connections_.push_back(connection);

    // Start listening for messages from this connection
    asio::co_spawn(io_context_, messageListener(connection), asio::detached);

    co_return;
}

asio::awaitable<void> TCPServer::listener()
{
    auto executor = co_await asio::this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), tcp_port_});
    std::cout << "Listening for TCP on port " << tcp_port_ << "\n";

    while (true)
    {
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(executor, handleAccept(std::move(socket)), asio::detached);
    }
}

asio::awaitable<void> TCPServer::sendMessage(TCPConnectionPtr connection, const std::string& message)
{
    co_await connection->send(message);
}

asio::awaitable<void> TCPServer::connect(const std::string_view address, const unsigned int port) 
{
    tcp::endpoint endpoint(asio::ip::make_address(address), port);
    tcp::socket socket(io_context_);

    co_await socket.async_connect(endpoint, asio::use_awaitable);
    asio::co_spawn(io_context_, handleAccept(std::move(socket)), asio::detached);

    co_return;
}