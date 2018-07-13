#pragma once
#include <plog/Severity.h>
#include <plog/Util.h>

namespace plog
{
    namespace detail
    {
        //////////////////////////////////////////////////////////////////////////
        // Stream output operators as free functions

        inline void operator<<(std::stringstream& stream, const char* data)
        {
            data = data ? data : "(null)";
            std::operator<<(stream, data);
        }

        inline void operator<<(std::stringstream& stream, const std::string& data)
        {
            plog::detail::operator<<(stream, data.c_str());
        }
    }

    class Record
    {
    public:
        Record(Severity severity, const char* func, size_t line, const char* file, const void* object)
            : m_severity(severity), m_tid(util::gettid()), m_object(object), m_line(line), m_func(func), m_file(file)
        {
          clock_gettime(CLOCK_REALTIME, &m_time);
        }

        //////////////////////////////////////////////////////////////////////////
        // Stream output operators

        Record& operator<<(char data)
        {
            char str[] = { data, 0 };
            return *this << str;
        }

        Record& operator<<(std::ostream& (*data)(std::ostream&))
        {
            m_message << data;
            return *this;
        }

        template<typename T>
        Record& operator<<(const T& data)
        {
            using namespace plog::detail;

            m_message << data;
            return *this;
        }

        //////////////////////////////////////////////////////////////////////////
        // Getters

        virtual const struct timespec& getTime() const
        {
            return m_time;
        }

        virtual Severity getSeverity() const
        {
            return m_severity;
        }

        virtual unsigned int getTid() const
        {
            return m_tid;
        }

        virtual const void* getObject() const { return m_object; }

        virtual size_t getLine() const
        {
            return m_line;
        }

        virtual const char* getMessage() const
        {
            m_messageStr = m_message.str();
            return m_messageStr.c_str();
        }

        virtual const char* getFunc() const
        {
            m_funcStr = util::processFuncName(m_func);
            return m_funcStr.c_str();
        }

        virtual const char* getFile() const
        {
            return m_file;
        }

    private:
        struct timespec         m_time;
        const Severity          m_severity;
        const unsigned int      m_tid;
        const void* const       m_object;
        const size_t            m_line;
        std::stringstream     m_message;
        const char* const       m_func;
        const char* const       m_file;
        mutable std::string     m_funcStr;
        mutable std::string   m_messageStr;
    };
}
