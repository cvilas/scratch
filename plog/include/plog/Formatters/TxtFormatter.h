#pragma once
#include <plog/Record.h>
#include <iomanip>

namespace plog
{
    class TxtFormatter
    {
    public:
        static std::string header()
        {
            return std::string();
        }

        static std::string format(const Record& record)
        {
          struct timespec time = record.getTime();
          tm t;
          ::localtime_r(&time.tv_sec, &t);

            std::stringstream ss;
            ss << t.tm_year + 1900 << "-" << std::setfill('0') << std::setw(2) << t.tm_mon + 1 << "-" << std::setfill('0') << std::setw(2) << t.tm_mday << " ";
            ss << std::setfill('0') << std::setw(2) << t.tm_hour << ":" << std::setfill('0') << std::setw(2) << t.tm_min << ":" << std::setfill('0') << std::setw(2) << t.tm_sec << "." << std::setfill('0') << std::setw(3) << time.tv_nsec/1000000 << " ";
            ss << std::setfill(' ') << std::setw(5) << std::left << severityToString(record.getSeverity()) << " ";
            ss << "[" << record.getTid() << "] ";
            ss << "[" << record.getFunc() << "@" << record.getLine() << "] ";
            ss << record.getMessage() << "\n";

            return ss.str();
        }
    };
}
