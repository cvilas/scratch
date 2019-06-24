// Efficient logging macro. Compile with -O2 on gcc > 4.8 for compact instruction sequence
// from https://stackoverflow.com/questions/19415845/a-better-log-macro-using-template-metaprogramming
//
// Others to look at:
// - https://github.com/SergiusTheBest/plog

#include <iostream>
#include <functional>

struct None
{
};

template<typename List>
struct LogData
{
    List list;
};

template<typename Begin, typename Value>
constexpr LogData<std::pair<Begin&&, Value&&>> operator<< (LogData<Begin>&& begin, Value&& value) noexcept
{
    return {{ std::forward<Begin>(begin.list), std::forward<Value>(value) }};
}

template<typename Begin, size_t n>
constexpr LogData<std::pair<Begin&&, const char*>> operator<< (LogData<Begin>&& begin, const char (&value)[n]) noexcept
{
    return {{ std::forward<Begin>(begin.list), value }};
}

using PfnManipulator = std::function<std::ostream&(std::ostream&)>;
//typedef std::ostream& (*PfnManipulator)(std::ostream&);

template<typename Begin>
constexpr LogData<std::pair<Begin&&, PfnManipulator>> operator<< (LogData<Begin>&& begin, PfnManipulator&& value) noexcept
{
    return {{ std::forward<Begin>(begin.list), value }};
}

template <typename Begin, typename Last>
void printList(std::ostream& os, std::pair<Begin, Last>&& data)
{
    printList(os, std::move(data.first));
    os << data.second;
}

inline void printList(std::ostream& os, None)
{ }

template<typename List>
void Log(const char* file, int line, LogData<List>&& data)
{
    std::cout << file << ":" << line << ": ";
    printList(std::cout, std::move(data.list));
    std::cout << std::endl;
}

#define LOG(msg) (Log(__FILE__, __LINE__, LogData<None>() << msg))

int main()
{
  LOG("Hello" << " World!");
  return 0;
}
