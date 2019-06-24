#pragma once

#include <chrono>
#include <string>
#include <fastrtps/qos/QosPolicies.h>

namespace test
{
struct MyTopicAttributes
{
  static constexpr char TOPIC[] = "my_topic";
  static constexpr char DATA_TYPE[] = "MyDataType";
  static constexpr eprosima::fastrtps::ReliabilityQosPolicyKind RELIABILITY_QOS = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
  static constexpr eprosima::fastrtps::DurabilityQosPolicyKind_t DURABILITY_QOS = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
  static constexpr int32_t DEFAULT_NUM_INSTANCES = 2;
  static constexpr int32_t DEFAULT_NUM_SAMPLES_PER_INSTANCE = 0;
  static constexpr uint32_t DEFAULT_THROUGHPUT_BYTES_PER_PERIOD = 200;
  static constexpr std::chrono::milliseconds DEFAULT_THROUGHPUT_PERIOD = std::chrono::milliseconds(1000);
  static constexpr std::chrono::milliseconds DEFAULT_DATA_PERIOD = std::chrono::milliseconds(100);
};
}
