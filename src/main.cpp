#include <iostream>

#include "echoserver.hpp"
#include "logserver.hpp"

int main(int argc, char** argv) {

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <server-type>" << " <port>" << "\n";
        return 1;
    }

    std::string serverType = argv[1];
    unsigned short port = std::stoi(argv[2]);

    if (serverType == "tcp")
    {
        EchoServer server(port);
        server.start();
    }
    else if (serverType == "udp")
    {
        LogServer server(port);
        server.start();
    }
    else
    {
        std::cerr << "Invalid server type: " << serverType << "\n";
        return 1;
    }

    return 0;
}