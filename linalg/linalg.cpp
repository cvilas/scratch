// g++ -std=c++20 -Wall -Wextra -pedantic linalg.cpp -o /tmp/linalg

#include "linalg.hpp"

#include <iostream>
#include <type_traits>

using namespace linalg;

struct World : Frame {};
struct Base : Frame {};
struct EndEffector : Frame {};
struct ToolCenterPoint : PointTag {};
struct Target : PointTag {};

int main() {
    constexpr auto sum = Vector<World>{{1., 2., 3.}} + Vector<World>{{4., 5., 6.}};
    static_assert(sum.coordinates() == Coordinates{5., 7., 9.});

    const auto C_WB = Rotation<World, Base>::from_matrix(
        {{{0., -1., 0.}, {1., 0., 0.}, {0., 0., 1.}}});
    const auto T_WB = Transform<World, Base>{C_WB, Displacement<World, World, Base>{{1., 2., 0.}}};
    const auto T_BE = Transform<Base, EndEffector>{
        Rotation<Base, EndEffector>{}, Displacement<Base, Base, EndEffector>{{1., 0., 0.}}};
    const auto T_WE = T_WB * T_BE;

    const auto tool_E = Position<EndEffector, ToolCenterPoint>{{1., 0., 0.}};
    const auto tool_W = T_WE * tool_E;
    static_assert(std::is_same_v<decltype(T_WE * tool_E), Position<World, ToolCenterPoint>>);

    const auto target_W = Position<World, Target>{{2., 5., 0.}};
    const auto tool_to_target_W = target_W - tool_W;
    const auto tool_to_target_E = T_WE.inverse() * tool_to_target_W;

    const auto& p = tool_W.coordinates();
    const auto& d = tool_to_target_E.coordinates();
    std::cout << "Tool in world: (" << p[0] << ", " << p[1] << ", " << p[2] << ")\n"
              << "Tool-to-target displacement in end effector: ("
              << d[0] << ", " << d[1] << ", " << d[2] << ")\n";

    // These expressions are deliberately ill-formed; see tests/test_linalg.py.
    // T_BE * T_WB;                         // Adjacent frames do not match.
    // T_WE * Position<Base, Target>{};     // Position is in the wrong frame.
    // tool_W + target_W;                  // Positions cannot be added.
    // target_W + tool_to_target_W;        // Displacement starts at the tool.
}
