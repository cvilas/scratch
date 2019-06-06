#include "mpscq.h"
#include <iostream>
#include <memory>

int main()
{
  mpscq<int, 10> q;

  while (q.push(1))
  {
    std::cout << "pushed " << q.count() << "\n";
  }

  while(q.count() > 0)
  {
    auto t = q.pop();
    if(std::get<0>(t))
    {
      auto p = std::get<1>(t);
      std::cout << p << " " << q.count() << "\n";
    }
  }
  return 0;
}
