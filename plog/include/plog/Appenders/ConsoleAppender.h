#pragma once
#include <plog/Appenders/IAppender.h>
#include <iostream>
#include <mutex>

namespace plog
{
    template<class Formatter>
    class ConsoleAppender : public IAppender
    {
    public:
        ConsoleAppender() : m_isatty(!!::isatty(::fileno(stdout))) {}

        virtual void write(const Record& record)
        {
            std::string str = Formatter::format(record);
            std::lock_guard<std::mutex> lock(writerMutex_);
            writestr(str);
        }

    protected:
        void writestr(const std::string& str)
        {
            std::cout << str << std::flush;
        }

    private:

    protected:
        std::mutex writerMutex_;
        const bool  m_isatty;
    };
}
