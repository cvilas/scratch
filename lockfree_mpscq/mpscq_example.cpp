#include "mpscq.h"
#include <iostream>
#include <thread>

/// \todo
/// - make many producer threads
/// - make one consumer thread
/// - show producer threads writing and consumer thread printing

constexpr size_t Q_LEN = 10;
static mpscq<int, Q_LEN> s_queue;
static std::atomic_bool s_exit = false;
static std::atomic_int s_value = 0;

void producer()
{
  while (!s_exit)
  {
    auto b = s_queue.push(++s_value);
    if(!b)
    {
      std::cout << "producer: queue full\n";
    }
  }
}

void consumer()
{
  while(!s_exit)
  {
    if(s_queue.count() > 0)
    {
      auto t = s_queue.pop();
      if(t.has_value())
      {
        std::cout << "consumer: value: " << t.value() << "\n";
      }
      else
      {
        std::cout << "consumer: queue busy\n";
      }
    }
    else
    {
      std::cout << "consumer: queue empty\n";
    }
  }
}

int main()
{
  std::thread();
  return 0;
}
