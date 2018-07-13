//
// Performance - measures time per a log call.
//

#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>

enum
{
    Console = 1
};

bool timespecDiff (struct timespec end, struct timespec start, struct timespec& diff) throw();

int main()
{
    // Initialize the logger that will be measured.
    plog::init(plog::debug, "Performance.txt");

    // Initialize the logger for printing info messages.
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init<Console>(plog::debug, &consoleAppender);

    LOGI_(Console) << "Test started";

    struct timespec startTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    const int kCount = 50000;

    // Performance measure loop.
    for (int i = 0; i < kCount; ++i)
    {
        LOGD << "Hello log!";
    }

    struct timespec finishTime;
    clock_gettime(CLOCK_REALTIME, &finishTime);

    struct timespec timeDiff;
    timespecDiff(finishTime, startTime, timeDiff);
    LOGI_(Console) << "Test finished: "
                   << ((static_cast<double>(timeDiff.tv_sec) * 1000.) + (static_cast<double>(timeDiff.tv_nsec)/1000.)) / kCount
                   << " microsec per call";

    return 0;
}

//--------------------------------------------------------------------------
bool timespecDiff (struct timespec end, struct timespec start, struct timespec& diff) throw()
//--------------------------------------------------------------------------
{
    static long nano = 1000000000;

    // from http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
    // Perform the carry for the later subtraction by updating y.
    if (end.tv_nsec < start.tv_nsec)
    {
        long int nsec = (start.tv_nsec - end.tv_nsec) / nano + 1;
        start.tv_nsec -= nano * nsec;
        start.tv_sec += nsec;
    }
    if (end.tv_nsec - start.tv_nsec > nano)
    {
        long int nsec = (end.tv_nsec - start.tv_nsec) / nano;
        start.tv_nsec += nano * nsec;
        start.tv_sec -= nsec;
    }

    // Compute the time remaining to wait. tv_usec is certainly positive.
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;

    // Return false if result is negative.
    return ( end.tv_sec < start.tv_sec ) ? (false) : (true);
}
