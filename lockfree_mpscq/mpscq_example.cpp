#include "mpscq.h"
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>

struct HeavyObject
{
  HeavyObject(uint32_t val = 0) { value[0] = val; }
  uint32_t value[128];
};

constexpr size_t Q_LEN = 1000;
static mpscq<HeavyObject, Q_LEN> s_queue;
static std::atomic_bool s_exit = false;
static std::atomic_uint s_value = 0;

void onSignal(int signum)
{
  std::cout << "Received signal " << strsignal(signum) << std::endl;
  s_exit = true;
}

void producer()
{
  while (!s_exit) {
    HeavyObject obj(s_value.load());
    auto pushed = s_queue.tryPush(std::move(obj));
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    std::this_thread::yield();
    if (pushed) {
      std::cout << std::hex << std::this_thread::get_id() << " produced - value: " << s_value << "\n";
      ++s_value;
    }
  }
}

void consumer()
{
  while (!s_exit) {
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    if (s_queue.count() > 0) {
      auto t = s_queue.tryPop();
      if (t.has_value()) {
        auto val_now = t.value().value[0];
        std::cout << "consumer - value: " << val_now << "\n";
      }
    }
  }
}

int main()
{
  signal(SIGINT, onSignal);
  signal(SIGTERM, onSignal);

  std::thread t1(producer);
  std::thread t2(producer);
  consumer();
  t1.join();
  t2.join();
  return 0;
}
