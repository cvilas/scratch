// From C++ Weekly - Ep 540 - Creating New Types with C++26's Reflection
// https://youtu.be/ZBqTr6YRROA
// gcc -O3 -std=c++26 -freflection ..

#include <algorithm>
#include <meta>
#include <string_view>

template <std::size_t LEN>
struct static_string {
  char data[LEN];
  consteval static_string(const char (&input)[LEN]) {
    std::copy(std::begin(input), std::end(input), std::begin(data));
  }
};

template <static_string Name, typename T>
struct pair {
  using type = T;
  static constexpr auto value = Name;
};

template <static_string ... Names>
consteval auto named_tuple(auto && ... values) {
  struct tuple;

  consteval {
    auto member = std::vector<std::meta::info>{
      std::meta::data_member_spec(^^std::remove_cvref_t<decltype(values)>, {.name = Names.data})...
    };
    std::meta::define_aggregate(^^tuple, members);
  }
  return tuple{std::forward<decltype(values)>(values)...};
}

auto generate_data() {
  return named_tuple<"key", "value">(std::string_view{"Jason"}, 42);
}

int main() {
  // cnttp

  const auto [key, value] = generate_data();

  return generate_data().value + value;
}


