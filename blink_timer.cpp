/// BlinkerTimer: Globally Synchronized Periodic Timer (C++23)
/// 
/// Executes a callback function at a fixed, configurable period, with timer events precisely 
/// aligned to wall-clock time. If multiple timers are started anywhere in the world with the 
/// same period (and their system clocks are synchronized to network time), their callbacks will 
/// fire at the exact same absolute times—regardless of when each timer instance was started.
///
/// compile with: g++ -std=c++23 -o blink_timer blink_timer.cpp
///
#include <chrono>
#include <functional>
#include <thread>
#include <print>

class BlinkerTimer {
public:
    using Clock = std::chrono::system_clock;
    using Duration = std::chrono::milliseconds;
    using Callback = std::function<void()>;

    BlinkerTimer(Duration period, Callback cb) {
        worker_ = std::jthread([period, callback = std::move(cb)](std::stop_token stoken) {
            const auto epoch = Clock::time_point{};
            while (!stoken.stop_requested()) {
                const auto elapsed = Clock::now() - epoch;
                const auto cycles = std::chrono::duration_cast<Duration>(elapsed) / period + 1;
                const auto next_tick = epoch + (cycles * period);
                std::this_thread::sleep_until(next_tick);
                callback();
            }
        });
    }

private:
    std::jthread worker_;
};

int main(int argc, const char* argv[]) {
  auto period = std::chrono::milliseconds(1000);
  if(argc == 2) {
    period = std::chrono::milliseconds(std::stoi(argv[1]));
  }
    
  BlinkerTimer timer(period, []() {
    const auto now = std::chrono::system_clock::now();
    std::println("Blink! {}", now);
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  return 0;
}
