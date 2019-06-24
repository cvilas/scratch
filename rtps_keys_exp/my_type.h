#pragma once

#include <fastcdr/FastCdr.h>
#include <cinttypes>
#include <ostream>

namespace test
{

/// Just a test datatype
struct MyType
{
  uint32_t id; //!< I want to utilize this as a DDS 'key'
  float x;
  float y;
  float z;
};

//---------------------------------------------------------------------------------------------------------------------
inline eprosima::fastcdr::FastCdr& operator<<(eprosima::fastcdr::FastCdr& ser, const MyType& data)
//---------------------------------------------------------------------------------------------------------------------
{
  ser << data.id << data.x << data.y << data.z;
  return ser;
}

//---------------------------------------------------------------------------------------------------------------------
inline eprosima::fastcdr::FastCdr& operator>>(eprosima::fastcdr::FastCdr& ser, MyType& data)
//---------------------------------------------------------------------------------------------------------------------
{
  ser >> data.id >> data.x >> data.y >> data.z;
  return ser;
}

//---------------------------------------------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const MyType& data)
//---------------------------------------------------------------------------------------------------------------------
{
  os << "{id=" << data.id << "; position=" << data.x << "," << data.y << "," << data.z << ";}";
  return os;
}

}
