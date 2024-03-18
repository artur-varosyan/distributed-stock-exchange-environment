#ifndef CSV_WRITER_HPP
#define CSV_WRITER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "csvprintable.hpp"

class CSVWriter
{
public:

    /** Writes the vector of printable items to the given CSV file. */
    static void writeToFile(std::string path, const std::vector<CSVPrintablePtr>& items)
    {
        std::cout << "Writing " << items.size() << " items to " << path << " ..." << std::endl;

        // Open file at given path
        std::ofstream file(path);

        // Write headers
        file << items[0]->describeCSVHeaders() << std::endl;

        // Iterate through items and write to file
        for (const CSVPrintablePtr& item : items)
        {
            file << item->toCSV() << std::endl;
        }

        // Close file
        file.close();

        std::cout << "Finished writing " << items.size() << " items to " << path << std::endl;
    }
};

#endif