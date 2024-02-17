#ifndef LOGSERVER_HPP
#define LOGSERVER_HPP

#include <iostream>
#include <boost/asio.hpp>

#include "udpserver.hpp"

class LogServer : public UDPServer
{
public:

    LogServer(unsigned short port)
    : UDPServer(port)
    {
    }

    void handleBroadcast(std::string_view message) override
    {
        std::cout << "Received message: " << message << "\n";
    }
};

#endif