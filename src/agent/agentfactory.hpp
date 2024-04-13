#ifndef AGENT_FACTORY_HPP
#define AGENT_FACTORY_HPP

#include "agent.hpp"
#include "agenttype.hpp"
#include "../config/agentconfig.hpp"

#include "stockexchange.hpp"
#include "exampletrader.hpp"
#include "marketdatawatcher.hpp"
#include "traderzic.hpp"
#include "tradershvr.hpp"

class AgentFactory
{
public:

    AgentFactory() = delete;

    /** Creates a new instance of an agent given a configuration and returns a pointer to it. */
    static std::shared_ptr<Agent> createAgent(NetworkEntity *network_entity, AgentConfigPtr config)
    {
        switch (config->type)
        {
            case AgentType::STOCK_EXCHANGE: 
            {
                std::shared_ptr<Agent> agent (new StockExchange{network_entity, std::static_pointer_cast<ExchangeConfig>(config)});
                return agent;
            }
            case AgentType::TRADER_EXAMPLE:
            {
                std::shared_ptr<Agent> agent (new ExampleTrader{network_entity, std::static_pointer_cast<TraderConfig>(config)});
                return agent;
            }
            case AgentType::MARKET_WATCHER:
            {
                std::shared_ptr<Agent> agent (new MarketDataWatcher{network_entity, std::static_pointer_cast<TraderConfig>(config)});
                return agent;
            }
            case AgentType::TRADER_ZIC:
            {
                std::shared_ptr<Agent> agent (new TraderZIC{network_entity, std::static_pointer_cast<TraderConfig>(config)});
                return agent;
            }
            case AgentType::TRADER_SHVR:
            {
                std::shared_ptr<Agent> agent (new TraderShaver{network_entity, std::static_pointer_cast<TraderConfig>(config)});
                return agent;
            }
            default:
            {
                throw std::runtime_error("Failed to create agent. Unknown agent received");
            }
        }
    }

    /** Returns the agent type corresponding to the given XML tag. */
    static AgentType getAgentTypeForTag(std::string& xml_tag) {
        if (xml_tag_map.find(xml_tag) == xml_tag_map.end())
        {
            throw std::runtime_error("XML Configuration Error. Cannot identify the agent for tag: " + xml_tag);
        }

        return xml_tag_map.at(xml_tag);
    }

private:

    /** Map of XML tags and the corresponding agent types. */
    static inline const std::unordered_map<std::string, AgentType> xml_tag_map {
        {std::string{"exchange"}, AgentType::STOCK_EXCHANGE},
        {std::string{"watcher"}, AgentType::MARKET_WATCHER},
        {std::string{"zic"}, AgentType::TRADER_ZIC},
        {std::string{"shvr"}, AgentType::TRADER_SHVR}
    };

};

#endif