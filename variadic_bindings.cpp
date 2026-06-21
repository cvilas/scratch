// C++ Weekly - Ep 485 - Variadic Structured Bindings in C++26 
// https://youtu.be/qIDFyhtUMnQ
//
// g++ -O3 -std=c++26 -o variadic_bindings variadic_bindings.cpp
// clang++ -stdlib=libc++ -std=c++26 -o variadic_bindings variadic_bindings.cpp

#include <print>
#include <ranges>
#include <string_view>
#include <vector>

constexpr void inspect(const auto& obj) {
    const auto &[...values] = obj; // <-- VSB

    auto byte_size = []<typename T>(const T& v) -> std::size_t {
        if constexpr (std::ranges::contiguous_range<T> && std::ranges::sized_range<T>)
            return std::ranges::size(v) * sizeof(*std::ranges::data(v));
        else
            return sizeof(v);
    };

    std::println("Num elements: {}", sizeof...(values));
    (std::println("value: {}, type:{}, size: {} bytes", values, typeid(values).name(), byte_size(values)), ...); // <-- Fold expression
}

int main() {
    struct MyData{
        int i;
        double f;
        std::string_view str;
        std::vector<double> v;
    };

    const auto m = MyData{2, 3.14, "hello", {1, 1.4142, 3, 5., 6.}};
    inspect(m);
}

/*
Output:
Num elements: 4
value: 2, type:i, size: 4 bytes
value: 3.14, type:d, size: 8 bytes
value: hello, type:NSt3__117basic_string_viewIcNS_11char_traitsIcEEEE, size: 5 bytes
value: [1, 1.4142, 3, 5, 6], type:NSt3__16vectorIdNS_9allocatorIdEEEE, size: 40 bytes
*/