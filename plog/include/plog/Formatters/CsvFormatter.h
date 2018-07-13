#pragma once
#include <plog/Record.h>
#include <iomanip>

namespace plog
{
    class CsvFormatter
    {
    public:
        static std::string header()
        {
            return "Date;Time;Severity;TID;This;Function;Message\n";
        }

        static std::string format(const Record& record)
        {
            struct timespec time = record.getTime();
            tm t;
            ::localtime_r(&time.tv_sec, &t);

            std::stringstream ss;
            ss << t.tm_year + 1900 << "/" << std::setfill('0') << std::setw(2) << t.tm_mon + 1 << "/" << std::setfill('0') << std::setw(2) << t.tm_mday << ";";
            ss << std::setfill('0') << std::setw(2) << t.tm_hour << ":" << std::setfill('0') << std::setw(2) << t.tm_min << ":" << std::setfill('0') << std::setw(2) << t.tm_sec << "." << std::setfill('0') << std::setw(3) << time.tv_nsec/10000000 << ";";
            ss << severityToString(record.getSeverity()) << ";";
            ss << record.getTid() << ";";
            ss << record.getObject() << ";";
            ss << record.getFunc() << "@" << record.getLine() << ";";

            std::string message = record.getMessage();

            if (message.size() > kMaxMessageSize)
            {
                message.resize(kMaxMessageSize);
                message.append("...");
            }

            std::stringstream split(message);
            std::string token;

            while (!split.eof())
            {
                std::getline(split, token, '"');
                ss << "\"" << token << "\"";
            }

            ss << "\n";

            return ss.str();
        }

        static const size_t kMaxMessageSize = 32000;
    };
}
