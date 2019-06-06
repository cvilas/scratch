#include "mpscq.h"
#include <iostream>
#include <thread>
#include <csignal>
#include <cstring>
#include <chrono>

/// \todo
/// - make many producer threads
/// - make one consumer thread
/// - show producer threads writing and consumer thread printing

struct HeavyObject
{
  HeavyObject(uint32_t val=0)
  {
    value[0] = val;
  }
  uint32_t value[128];
};

constexpr size_t Q_LEN = 10;
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
  while (!s_exit)
  {
      HeavyObject obj(s_value.load());
      auto b = s_queue.push(std::move(obj));
      //std::this_thread::sleep_for(std::chrono::microseconds(1));
      std::this_thread::yield();
    if(b)
    {
        ++s_value;
        std::cout << "produced\n";
    }
  }
}

void consumer()
{
  uint32_t val = 0;
  while(!s_exit)
  {
      std::this_thread::yield();
      //std::this_thread::sleep_for(std::chrono::microseconds(1));
    if(s_queue.count() > 0)
    {
      auto t = s_queue.pop();
      if(t.has_value())
      {
          auto val_now = t.value().value[0];
          //if(val != val_now)
           {
              std::cout << "consumer: value: " << val << " != "<< val_now << "\n";
           }
          val = val_now+1;
      }
    }
  }
}

int main()
{
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

  std::thread t1(producer);
  consumer();
  t1.join();
  return 0;
}
