#include "writer.h"
#include "reader.h"
#include "my_type.h"

#include <fastrtps/rtps/RTPSDomain.h>

#include <iostream>

//=====================================================================================================================
void reader_callback(uint8_t* const data, uint32_t dataLength)
//=====================================================================================================================
{
  eprosima::fastcdr::FastBuffer buffer(reinterpret_cast<char* const>(data), dataLength);
  eprosima::fastcdr::FastCdr des(buffer);
  auto pvalue = std::make_unique<test::MyType>();
  des >> *pvalue;
  std::cout << *pvalue << "\n";
}

//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
  bool exit_flag = false;
  std::thread writer_thread([&exit_flag]()
  {
    test::Writer writer;
    test::MyType data;
    while(!exit_flag)
    {
      eprosima::fastcdr::FastBuffer buffer;
      eprosima::fastcdr::FastCdr serdes(buffer);
      serdes << data;
      writer.write(reinterpret_cast<uint8_t*>(buffer.getBuffer()), serdes.getSerializedDataLength());
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


