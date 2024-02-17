#include <iostream>

#include "tcpserver.hpp"

int main() {
    std::cout << "Starting a tcp server" << "\n";

    TCPServer server(8080);
    server.start();

    return 0;
}