#ifndef ECHOSERVER_HPP
#define ECHOSERVER_HPP

#include "tcpserver.hpp"

class EchoServer : public TCPServer
{
public:
    EchoServer(unsigned short port)
    : TCPServer(port)
    {
    }

    std::string handleMessage(std::string_view message) override
    {
        return std::string(message);
    }
};


#endif