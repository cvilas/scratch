// Coordinate-frames aware linear algebra
// g++ -std=c++23 -o linalg linalg.cpp

#include <print>
#include <string_view>
#include <array>
#include <typeinfo>
#include <concepts>
#include <algorithm>
#include <cstdlib>

struct Frame {};

template<typename T>
concept FrameType = std::derived_from<T, Frame>;

template <typename T>
constexpr auto getTypeName() -> std::string_view {
  constexpr std::string_view FUNCTION_NAME = __PRETTY_FUNCTION__;
  constexpr std::string_view PREFIX = "T = ";
  constexpr auto START = FUNCTION_NAME.find(PREFIX) + PREFIX.size();
  constexpr auto END = FUNCTION_NAME.find_first_of("];", START);
  return FUNCTION_NAME.substr(START, END - START);
}

template<FrameType Frame>
struct Vector : public std::array<double, 3> {

    constexpr auto operator+(const Vector<Frame>& other) const -> Vector<Frame> {
        Vector<Frame> result;
        std::transform(this->begin(), this->end(), other.begin(), result.begin(), std::plus<>());
        return result;
    }
};

template<FrameType From, FrameType To>
struct Rotation {

    template<FrameType From2, FrameType To2> requires std::same_as<From, To2>
    constexpr auto operator*(const Rotation<From2, To2>& other) const -> Rotation<From2, To>{
        return {};
    }

    template<FrameType From2> requires std::same_as<From, From2>
    constexpr auto operator*(const Vector<From2>& v) const -> Vector<To>{
        return {};
    }

};

struct World : Frame{};
struct Base : Frame{};
struct EndEffector : Frame{};
struct ToolCenterPoint : Frame{};

int main() {

    constexpr auto a = Vector<World>({1., 2., 3.});
    constexpr auto b = Vector<World>({4., 5., 6.});
    constexpr auto c = a + b;
    std::println("{}", c);
    std::println("c is {}", getTypeName<decltype(c)>());

    constexpr auto r1 = Rotation<Base, World>{} * Rotation<EndEffector, Base>{};
    std::println("r1 is {}", getTypeName<decltype(r1)>());

    constexpr auto v1 = Rotation<Base, World>{} * Vector<Base>{};
    std::println("v1 is {}", getTypeName<decltype(v1)>());

    return EXIT_SUCCESS;
}