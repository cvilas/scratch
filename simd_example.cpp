// std::simd example
// g++ -O3 -std=c++23 -o simd_example simd_example.cpp

#include <experimental/simd>
#include <print>

namespace stdx = std::experimental;
 
void print_simd(stdx::native_simd<float> s) {
    std::println("Width: {}", s.size());
    for (unsigned i = 0; i < s.size(); ++i) {
        std::print("{} ", float(s[i]));
    }
    std::println("");
}

int main() {
    stdx::native_simd<float> a([](int i) { return float(i + 1); });
    stdx::native_simd<float> b([](int i) { return float(i + 5); });
    auto c = a + b;
    print_simd(a);
    print_simd(b);
    print_simd(c);
}