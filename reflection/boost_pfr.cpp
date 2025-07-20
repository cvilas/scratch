// Reflection with Boost PFR (C++17)
// g++ -std=c++17 -o boost_pfr boost_pfr.cpp

#include <string>
#include <variant>
#include <array>
#include <chrono>
#include <iostream>

#include "boost/pfr.hpp"

//-------------------------------------------------------------------------------------------------
struct State {
  std::string name;
  std::chrono::system_clock::time_point timestamp{};
  std::array<double, 3> position{};
};

//-------------------------------------------------------------------------------------------------
struct Sit {
  float speed{ 0.0F };
};

//-------------------------------------------------------------------------------------------------
struct Stand {
  float speed{ 0.0F };
};

//-------------------------------------------------------------------------------------------------
struct Walk {
  float forward_speed{ 0.0F };
  float lateral_speed{ 0.0F };
  float turn_speed{ 0.0F };
};

using Command = std::variant<Sit, Stand, Walk>;

//-------------------------------------------------------------------------------------------------
struct Position {
  double x{};
  double y{};
  double z{};
};

//-------------------------------------------------------------------------------------------------
struct Quaternion {
  double x{};
  double y{};
  double z{};
  double w{};
};

//-------------------------------------------------------------------------------------------------
struct PoseStamped {
  std::chrono::system_clock::time_point timestamp{};
  Position position{};
  Quaternion orientation{};
};

int main() {
    const auto state = State{ 
        .name = "/robot_name", 
        .timestamp = std::chrono::system_clock::now(), 
        .position = {0., 1., 3.}
        };

    const auto pose = PoseStamped{
        .timestamp = std::chrono::system_clock::now(),
        .position = {4., 5., 6.},
        .orientation = {0., 1., 2., 3.}
    }; 

    boost::pfr::for_each_field(state, [](const auto& f){ std::cout << boost::pfr::io(f) << "\n"; }); 
    std::cout << "\n";    

    boost::pfr::for_each_field(pose, [](const auto& f){ std::cout << boost::pfr::io(f) << "\n"; }); 
    std::cout << "\n";    
}