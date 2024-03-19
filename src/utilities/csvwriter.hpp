#ifndef CSV_WRITER_HPP
#define CSV_WRITER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread>

#include "syncqueue.hpp"
#include "csvprintable.hpp"

class CSVWriter
{
public:

    CSVWriter() = delete;

    CSVWriter(std::string path)
    : path_{path},
      file_{path}
    {
    };

    ~CSVWriter()
    {
        stop();
    };

    /** Stops the CSV writer and closes the file. */
    void stop()
    {
        file_.close();
    }

    /** Writes the given item as a CSV row immediately. */
    void writeRow(CSVPrintablePtr item) {
        if (!started_) 
        {
            file_ << item->describeCSVHeaders() << std::endl;
            started_ = true;
        }
        file_ << item->toCSV() << std::endl;
    }

private:

    std::string path_;
    std::ofstream file_;
    bool started_ = false;
};

typedef std::shared_ptr<CSVWriter> CSVWriterPtr;

#endif