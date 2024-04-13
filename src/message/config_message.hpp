#ifndef CONFIG_MESSAGE_HPP
#define CONFIG_MESSAGE_HPP

#include <unordered_map>

#include "message.hpp"
#include "../agent/agenttype.hpp"
#include "../config/agentconfig.hpp"

/** The message sent by the OrchestratorAgent to each SimulationAgent to configure that node. */
class ConfigMessage : public Message
{
public:

    ConfigMessage() : Message(MessageType::CONFIG) {};

    /** The IPv4 address of the agent. */
    std::string my_addr;

    /** The type of the agent. */
    AgentType agent_type;

    /** Agent-specific configuration object. */
    AgentConfigPtr config;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<Message>(*this);
        ar & my_addr;
        ar & agent_type;
        ar & config;
    }

};

typedef std::shared_ptr<ConfigMessage> ConfigMessagePtr;

#endif