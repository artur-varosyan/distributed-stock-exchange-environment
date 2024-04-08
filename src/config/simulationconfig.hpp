#ifndef SIMULATION_CONFIG_HPP
#define SIMULATION_CONFIG_HPP

#include <memory>

#include "agentconfig.hpp"
#include "exchangeconfig.hpp"
#include "traderconfig.hpp"

/** Configuration object for a single simulation. */
class SimulationConfig : std::enable_shared_from_this<SimulationConfig>
{
public:

    SimulationConfig() = delete;

    SimulationConfig(std::vector<ExchangeConfigPtr> exchange_configs, std::vector<AgentConfigPtr> trader_configs) 
    : exchange_configs_{exchange_configs},
      trader_configs_{trader_configs}
    {
    }

    const std::vector<ExchangeConfigPtr> exchanges() { return exchange_configs_; }
    const std::vector<AgentConfigPtr> traders() { return trader_configs_; }

private:

    std::vector<ExchangeConfigPtr> exchange_configs_;
    std::vector<AgentConfigPtr> trader_configs_;
};

typedef std::shared_ptr<SimulationConfig> SimulationConfigPtr;

#endif