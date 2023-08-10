// This snippet tries to reproduce results in https://pharr.org/matt/blog/2019/11/03/difference-of-floats
// However, note that when using double precision math, the results are again significantly different
// Compile as g++ -std=c++20 -o dof dof.cpp

#include <cmath>
#include <iostream>

inline consteval auto dop(auto a, auto b, auto c, auto d) {
    auto cd = c * d;
    auto err = std::fma(-c, d, cd);
    auto dop = std::fma(a, b, -cd);
    return dop + err;
}

template<typename T>
struct Vector {
    T x;
    T y;
    T z;
};

template<typename T>
constexpr auto operator<<(std::ostream& os, const Vector<T>& v) -> std::ostream& {
    os << v.x << ", " << v.y << ", " << v.z;
    return os;
}

template<typename T>
inline consteval auto cross(const Vector<T> &v1, const Vector<T> &v2) -> Vector<T> {
    T v1x = v1.x, v1y = v1.y, v1z = v1.z;
    T v2x = v2.x, v2y = v2.y, v2z = v2.z;
    return {v1y * v2z - v1z * v2y, v1z * v2x - v1x * v2z, v1x * v2y - v1y * v2x};
}

template<typename T>
inline consteval auto cross_dop(const Vector<T> &v1, const Vector<T> &v2) -> Vector<T> {
    return {dop(v1.y, v2.z, v1.z, v2.y), dop(v1.z, v2.x, v1.x, v2.z), dop(v1.x, v2.y, v1.y, v2.x)};
}

int main() {
    constexpr auto a = 33962.035;
    constexpr auto b = -30438.8;
    constexpr auto c = 41563.4;
    constexpr auto d = -24871.969;
    const auto dop_naive = a*b - c*d;
    const auto dop_correct = dop(a,b,c,d);
    std::cout << std::fixed << "dop_naive:" << dop_naive << ",\ndop_correct:" << dop_correct << "\n";
    
    constexpr auto v1 = Vector<double>{33962.035, 41563.4, 7706.415};
    constexpr auto v2 = Vector<double>{-24871.969, -30438.8, -5643.727};

    const auto cross_naive = cross(v1,v2);
    const auto cross_correct = cross_dop(v1,v2);
    std::cout << std::fixed << "cross_naive:" << cross_naive << ",\ncross_correct:" << cross_correct << "\n";
    
    return EXIT_SUCCESS;
}