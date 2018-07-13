#pragma once
#include <plog/Appenders/ConsoleAppender.h>

namespace plog
{
    template<class Formatter>
    class ColorConsoleAppender : public ConsoleAppender<Formatter>
    {
    public:
        ColorConsoleAppender() {}

        virtual void write(const Record& record)
        {
            std::string str = Formatter::format(record);
            std::lock_guard<std::mutex> lock(this->writerMutex_);

            setColor(record.getSeverity());
            this->writestr(str);
            resetColor();
        }

    private:
        void setColor(Severity severity)
        {
            if (this->m_isatty)
            {
                switch (severity)
                {
                case fatal:
                    std::cout << "\x1B[97m\x1B[41m"; // white on red background
                    break;

                case error:
                    std::cout << "\x1B[91m"; // red
                    break;

                case warning:
                    std::cout << "\x1B[93m"; // yellow
                    break;

                case debug:
                case verbose:
                    std::cout << "\x1B[96m"; // cyan
                    break;
                default:
                    break;
                }
            }
        }

        void resetColor()
        {
            if (this->m_isatty)
            {
                std::cout << "\x1B[0m\x1B[0K";
            }
        }

    private:
    };
}
