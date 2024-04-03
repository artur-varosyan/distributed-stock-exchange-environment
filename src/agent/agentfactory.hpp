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
    static std::shared_ptr<Agent> createAgent(NetworkEntity *network_entity, AgentType type, AgentConfig *config)
    {
        switch (type)
        {
            case AgentType::STOCK_EXCHANGE: 
            {
                std::shared_ptr<Agent> agent (new StockExchange{network_entity, (ExchangeConfig*) config});
                std::cout << "tickers = " << ((ExchangeConfig*) config)->name << "\n";
                return agent;
            }
            case AgentType::TRADER_EXAMPLE:
            {
                std::shared_ptr<Agent> agent (new ExampleTrader{network_entity, (TraderConfig*) config});
                return agent;
            }
            case AgentType::MARKET_WATCHER:
            {
                std::shared_ptr<Agent> agent (new MarketDataWatcher{network_entity, (TraderConfig*) config});
                return agent;
            }
            case AgentType::TRADER_ZIC:
            {
                std::shared_ptr<Agent> agent (new TraderZIC{network_entity, (TraderConfig*) config});
                return agent;
            }
            case AgentType::TRADER_SHVR:
            {
                std::shared_ptr<Agent> agent (new TraderShaver{network_entity, (TraderConfig*) config});
                return agent;
            }
        }
    }
};

#endif