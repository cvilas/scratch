#pragma once
#include <plog/Record.h>
#include <plog/Util.h>

namespace plog
{
    class MessageOnlyFormatter
    {
    public:
        static std::string header()
        {
            return std::string();
        }

        static std::string format(const Record& record)
        {
            std::stringstream ss;
            ss << record.getMessage() << "\n";

            return ss.str();
        }
    };
}
