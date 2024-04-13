#ifndef CSV_PRINTABLE_HPP
#define CSV_PRINTABLE_HPP

#include <string>

#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class CSVPrintable
{
public:

    virtual ~CSVPrintable() = default;

    virtual std::string describeCSVHeaders() const = 0;
    virtual std::string toCSV() const = 0;

private:

    /** Enable serialisation of derived classes. */
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
};

typedef std::shared_ptr<CSVPrintable> CSVPrintablePtr;

#endif