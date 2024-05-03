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

    SimulationConfig(int repetitions, int time, std::vector<ExchangeConfigPtr> exchange_configs, std::vector<AgentConfigPtr> trader_configs) 
    : repetitions_{repetitions},
      time_{time},
      exchange_configs_{exchange_configs},
      trader_configs_{trader_configs}
    {
    }

    const std::vector<ExchangeConfigPtr> exchanges() { return exchange_configs_; }
    const std::vector<AgentConfigPtr> traders() { return trader_configs_; }
    const int repetitions() { return repetitions_; }
    const int time() { return time_; }

private:

    std::vector<ExchangeConfigPtr> exchange_configs_;
    std::vector<AgentConfigPtr> trader_configs_;
    int repetitions_;
    int time_;
};

typedef std::shared_ptr<SimulationConfig> SimulationConfigPtr;

#endif