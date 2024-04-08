#ifndef EXCHANGE_CONFIG_HPP
#define EXCHANGE_CONFIG_HPP

#include "agentconfig.hpp"

class ExchangeConfig : public AgentConfig
{
public:

    ExchangeConfig() = default;

    std::string name;
    std::vector<std::string> tickers;
    int connect_time;
    int trading_time;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<AgentConfig>(*this);
        ar & name;
        ar & tickers;
        ar & connect_time;
        ar & trading_time;
    }
};

typedef std::shared_ptr<ExchangeConfig> ExchangeConfigPtr;

#endif