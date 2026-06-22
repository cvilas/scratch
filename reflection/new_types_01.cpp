// From C++ Weekly - Ep 540 - Creating New Types with C++26's Reflection
// https://youtu.be/ZBqTr6YRROA
// gcc -O3 -std=c++26 -freflection ..

#include <algorithm>
#include <meta>

auto make_location() {
    struct location;

    consteval {
        auto members = std::vector<std::meta::info> {
            std::meta::data_member_spec(^^int, { .name = "x" }),
            std::meta::data_member_spec(^^int, { .name = "y" }),
        };
        std::meta::define_aggregate(^^location, members);
    }
    return location{};
}

using location = decltype(make_location());

int main() {
    auto loc = location{ .x = 1, .y = 2};
}
