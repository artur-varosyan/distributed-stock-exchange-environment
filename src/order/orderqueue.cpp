#include "orderqueue.hpp"

std::optional<OrderPtr> OrderQueue::find(int order_id)
{
    for (auto it = this->c.begin(); it != this->c.end(); ++it)
    {
        if ((*it)->id == order_id)
        {
            return *it;
        }
    }
    return std::nullopt;
};

std::optional<OrderPtr> OrderQueue::remove(int order_id)
{
    std::optional<OrderPtr> order = this->find(order_id);
    if (order.has_value())
    {
        this->c.erase(std::remove(this->c.begin(), this->c.end(), order.value()), this->c.end());
        return order;
    }
    
    return std::nullopt;
};