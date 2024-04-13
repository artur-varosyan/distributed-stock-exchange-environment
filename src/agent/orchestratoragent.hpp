#ifndef ORCHESTRATOR_AGENT_HPP
#define ORCHESTRATOR_AGENT_HPP

#include "agent.hpp"
#include "agenttype.hpp"
#include "../config/simulationconfig.hpp"
#include "../config/agentconfig.hpp"
#include "../config/exchangeconfig.hpp"
#include "../message/config_message.hpp"

class OrchestratorAgent : public Agent 
{
public: 

    OrchestratorAgent(NetworkEntity *network_entity, AgentConfigPtr config)
    : Agent(network_entity, config)
    {
    }

    /** Configures the simulation given a simulation configuration. */
    void configureSimulation(SimulationConfigPtr simulation)
    {
        configuration_thread_ = new std::thread([&](){

            // Initialise exchanges
            for (auto exchange_config : simulation->exchanges())
            {
                configureNode(exchange_config);
            }

            // Allow exchanges to initialise first
            std::this_thread::sleep_for(std::chrono::seconds(10));

            // Initialise traders
            for (auto trader_config : simulation->traders())
            {
                configureNode(trader_config);
            }
        });
    }

    /** Sends a config message to the simulation node at the given address. */
    void configureNode(AgentConfigPtr config)
    {
        std::cout << "Initialising agent: " << to_string(config->type) << " with addr: " << config->addr << "\n";
        this->connect(std::string(config->addr), std::to_string(config->agent_id), [=, this](){

            ConfigMessagePtr msg = std::make_shared<ConfigMessage>();
            msg->config = config.get();

            std::string agent_id = std::to_string(config->agent_id);
            this->sendMessageTo(agent_id, std::static_pointer_cast<Message>(msg));
        });
    }

private:

    /** Checks the type of the incoming message and makes a callback. */
    std::optional<MessagePtr> handleMessageFrom(std::string_view sender, MessagePtr message) override
    {
        std::cout << "Orchestrator received a message" << "\n";
        return std::nullopt;
    }

    /** Checks the type of the incoming broadcast and makes a callback. */
    void handleBroadcastFrom(std::string_view sender, MessagePtr message) override 
    {
        std::cout << "Orchestrator received a broadcast" << "\n";
    }

    std::thread* configuration_thread_;
};

#endif