#include "logman.h"
#include <boost/thread.hpp>

void foo()
{
    LOGMAN(logINFO) << "Some log message here.";
}

int main(int argc, char* argv[])
{
    FILELog::ReportingLevel() = logDEBUG;
    LogMan::Set("main2.cpp", 6, LogMan::ENABLED);
    boost::thread t1(foo);
    boost::thread t2(foo);
    t1.join();
    t2.join();
    return 0;
}
