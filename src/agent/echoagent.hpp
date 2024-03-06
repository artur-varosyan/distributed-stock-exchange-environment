#ifndef ECHO_AGENT_HPP
#define ECHO_AGENT_HPP

#include <iostream>

#include "agent.hpp"

class EchoAgent : public Agent
{
public:

    EchoAgent(asio::io_context& io_context)
    : Agent(io_context)
    {
    }

    virtual ~EchoAgent() = default;

    std::optional<Message> handleMessage(Message message) override
    {
        std::cout << "Received message from " << message.sender_id << "\n";
        return message;
    }

    void handleBroadcast(Message message) override
    {
        std::cout << "Received broadcast from " << message.sender_id << "\n";
        return;
    }

};

#endif