#ifndef ZIP_CONFIG_HPP
#define ZIP_CONFIG_HPP

#include "traderconfig.hpp"

class ZIPConfig : public TraderConfig
{
public:

    ZIPConfig() = default;

    double min_margin;
    unsigned int liquidity_interval;

private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<TraderConfig>(*this);
        ar & min_margin;
        ar & liquidity_interval;
    }

};

typedef std::shared_ptr<ZIPConfig> ZIPConfigPtr;

#endif