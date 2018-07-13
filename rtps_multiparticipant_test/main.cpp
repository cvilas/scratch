#include "writer.h"
#include "reader.h"
#include <fastrtps/rtps/RTPSDomain.h>

#include <iostream>

//=====================================================================================================================
void reader_callback(uint8_t* const data, uint32_t dataLength)
//=====================================================================================================================
{
    std::cout << *(uint32_t*)data << std::endl;
}

//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
  bool exit_flag = false;
  std::thread writer_thread([&exit_flag]()
  {
    test::Writer writer;
    uint32_t count = 0;
    while(!exit_flag)
    {
        count++;
        writer.write((uint8_t*)&count, sizeof(count));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  });

  std::thread reader_thread([&exit_flag]()
  {
    test::Reader reader(&reader_callback);
    while(!exit_flag)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  });

  writer_thread.join();
  reader_thread.join();
  return 0;
}


