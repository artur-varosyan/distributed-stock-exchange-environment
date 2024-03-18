#ifndef CSV_PRINTABLE_HPP
#define CSV_PRINTABLE_HPP

#include <string>

class CSVPrintable
{
public:

    virtual ~CSVPrintable() = default;

    virtual std::string describeCSVHeaders() const = 0;
    virtual std::string toCSV() const = 0;
};

typedef std::shared_ptr<CSVPrintable> CSVPrintablePtr;

#endif