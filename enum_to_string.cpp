// Build a map either at compile time or statically once at runtime for enums to strings.
// Reference: https://godbolt.org/z/jx6vzPjnW
// Build with g++ -std=c++23 -o enum_to_string enum_to_string.cpp


#include <utility>
#include <array>
#include <string_view>

namespace grape::enums {
namespace detail
{

template<auto E> 
consteval auto extract_enumerator_name() -> std::string_view {
    const std::string_view func_name = __PRETTY_FUNCTION__;

#if defined(__clang__)
    const std::string_view start_token = "[E = ";
    const std::string_view end_token = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
    const std::string_view start_token = "with auto E = ";
    const std::string_view end_token = ";";
#else
    #error "Unsupported compiler. Use GCC or Clang"
#endif
    const auto temp_args_list = func_name.substr(func_name.find(start_token) + start_token.size());
    if (temp_args_list.starts_with("(")) {
        return "";
    }

    const auto enum_name_end = temp_args_list.find(end_token);
    const auto enum_name_start = temp_args_list.rfind("::", enum_name_end) + 2;
    const auto enum_name_length = enum_name_end - enum_name_start;
    return temp_args_list.substr(enum_name_start, enum_name_length);
}

template<auto E>
struct NameStorage {
    static constexpr auto name = extract_enumerator_name<E>();
    
    static constexpr auto make_array() {
        std::array<char, name.size()> result{};
        for (std::size_t i = 0; i < name.size(); ++i) {
            result.at(i) = name.at(i);
        }
        return result;
    }
    
    static constexpr auto value = make_array();
};

template<auto E>
consteval auto extract_and_allocate_enumerator_name() -> std::string_view {
    return {NameStorage<E>::value.data(), NameStorage<E>::value.size()};
}

template<int N, int... Seq>
constexpr std::integer_sequence<int, N + Seq ...> add(std::integer_sequence<int, Seq...>) { return {}; }

template<int Min, int Max>
using make_integer_range = decltype(add<Min>(std::make_integer_sequence<int, Max-Min>()));

} // namespace detail

template <typename Enum> requires std::is_enum_v<Enum>
struct enum_range {
  static constexpr int min = 0;
  static constexpr int max = 1;
  // (max - min) must be less than UINT16_MAX.
};

template<typename Enum> requires std::is_enum_v<Enum>
constexpr auto enum_names_list =
(
    []<auto... Es>(std::integer_sequence<int, Es...>) {
        return std::array{
            detail::extract_and_allocate_enumerator_name<static_cast<Enum>(Es)>()...
        };
    }
)(detail::make_integer_range<enum_range<Enum>::min, enum_range<Enum>::max+1>{});

template<typename Enum> requires std::is_enum_v<Enum>
constexpr std::string_view enum_name(Enum val) {
    const auto i = static_cast<std::size_t>(val) - enum_range<Enum>::min;
    if( i < enum_names_list<Enum>.size() ){
        return enum_names_list<Enum>.at(i);
    }
    return "";
}

} // namespace grape::enums

// TODO
// - Document limitations
//   - If enum range is not specified with a specialisation for the type, the default is assumed (provide example)
//   - Values must not be aliased (provide example)

// ----
#include <iostream>

enum class number { one = 100, two = 200, three = 300 };

template <>
struct grape::enums::enum_range<number> {
  static constexpr int min = 100;
  static constexpr int max = 300;
};

enum class Color {Red=-2, Green, Blue, Black, White};
template <>
struct grape::enums::enum_range<Color> {
  static constexpr int min = -2;
  static constexpr int max = 2;
};

auto main() -> int {
    std::cout << "Compile time: " << grape::enums::enum_name(number::one) << "\n"; 
    std::cout << "Compile time: " << grape::enums::enum_name(Color::Red) << "\n"; 
    auto e = static_cast<std::underlying_type_t<Color>>(Color::Red);
    e += 2;
    std::cout << "Run time: " << grape::enums::enum_name(static_cast<Color>(e)) << "\n"; 
    std::cout << "All names:\n";
    for(const auto& e : grape::enums::enum_names_list<Color>){
        std::cout << e << "\n";
    }
    return 0;
}
