#include <iostream>

#include "echoserver.hpp"

int main() {
    std::cout << "Starting a tcp server" << "\n";

    EchoServer server(8080);
    server.start();

    return 0;
}