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

    std::optional<Message> handleMessageFrom(std::string_view sender, Message message) override
    {
        std::cout << "Received message from sender " << sender << " with id: "<< message.sender_id << "\n";
        return message;
    }

    void handleBroadcastFrom(std::string_view sender, Message message) override
    {
        std::cout << "Received broadcast from sender " << sender << " with id: "<< message.sender_id << "\n";
        return;
    }

};

#endif