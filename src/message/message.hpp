#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>


/** A message that can be sent between network entities. 
 * TODO: Consider using the Builder or Factory pattern to create messages.
*/
struct Message
{
public:

    enum class Type {
        INIT,
        CONFIG,
        MARKET_DATA
    };

    Message() = default;

    Message(Type type) : type{type} {}

    virtual ~Message() = default;

    /** Returns the size of the serialized messaged in bytes. */
    size_t getSerializedSize()
    {
        std::ostringstream oss;
        boost::archive::text_oarchive oa{oss};
        oa << *this;
        return oss.str().size();
    }

    int sender_id;
    std::vector<int> receiver_ids;
    Type type;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & type;

        ar & sender_id;
        ar & receiver_ids;
    }

};

#endif