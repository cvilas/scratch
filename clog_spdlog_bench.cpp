#include <iostream>
#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <source_location>
#include <format>

// build with
// g++ -std=c++23 -O3 -o clog_spdlog_bench clog_spdlog_bench.cpp -I ./spdlog/include

struct Logger{
    enum class Severity : uint8_t {Debug, Critical};
    constexpr auto toString(Severity s) const -> std::string_view {
        switch(s){
            case Severity::Debug: return "Debug";
            case Severity::Critical: return "Critical";
        }
    }
    explicit Logger(const std::string& name) : name_(name) {}
    void log(Severity s, const std::string& m, const std::source_location& l = std::source_location::current()) const {
        std::clog << std::format("[{}] [{}] [{}:{}] [{}] {}\n", std::chrono::system_clock::now(), name_, l.file_name(), l.line(), toString(s), m);
    }
    const std::string name_;
};

void benchmarkStdClog(int numIterations) {
    Logger logger("benchmark_clogger");
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numIterations; ++i) {
        logger.log(Logger::Severity::Debug, std::format("Log message {}", i));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "std::clog - Elapsed time: " << duration.count() << " milliseconds" << std::endl;
}

void benchmarkSpdlog(int numIterations) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("benchmark_slogger", console_sink);

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numIterations; ++i) {
        //logger->info("Log message {}", i);
        SPDLOG_LOGGER_INFO(logger, "Log message {}", i);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "spdlog - Elapsed time: " << duration.count() << " milliseconds" << std::endl;
}

int main(int argc, char* argv[]) {
    constexpr int numIterations = 100000;  // Number of log messages to output

    if(argc > 1) {
    // Benchmark std::clog
    benchmarkStdClog(numIterations);
    } else {
    // Benchmark spdlog
    //spdlog::set_pattern("[%H:%M:%S.%e] [%!] %v");
    benchmarkSpdlog(numIterations);
    }
    return 0;
}
