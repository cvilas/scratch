// C++ Reflection: A universal printer
// Lieven de Cock
// Overload 193 (A magazine of the ACCU), June 2026
//
// g++ -std=c++26 -freflection universal_print.cpp -o universal_print

#include <meta>
#include <format>
#include <print>

struct EnumFormatter {
    constexpr auto parse(auto& ctx) {
        return ctx.begin();
    }

    template<typename E> requires std::is_enum_v<E>
    auto format(const E& value, auto& ctx) const {
        std::string_view label("unknown");
        template for(constexpr auto& enu : std::define_static_array(std::meta::enumerators_of(^^E))) {
            if(value == [:enu:]) {
                label = std::meta::identifier_of(enu);
                break;
            }
        }
        auto out = ctx.out();
        std::format_to(out, "{}", label);
        return out;
    }
};

struct UniversalFormatter {
    constexpr auto parse(auto& ctx) { 
        return ctx.begin(); 
    }

    template<typename T>
    auto format(const T& t, auto& ctx) const {
        auto out = ctx.out();

        std::string_view typeName = "unnamed";
        if constexpr (has_identifier(^^T)) {
            typeName = identifier_of(^^T);
        }
        std::format_to(out, "{}{{", typeName);

        auto delim = [first = true, &out]() mutable {
            if (!first) {
                std::format_to(out, ", ");
            }
            first = false;
        };

        constexpr auto access_ctx = std::meta::access_context::unchecked();

        template for (constexpr auto base : define_static_array(bases_of(^^T, access_ctx))) {
            delim();
            std::format_to(out, "{}", (typename [: type_of(base) :] const&)(t));
        }

        template for (constexpr auto member : define_static_array(nonstatic_data_members_of(^^T, access_ctx))) {
            delim();
            std::format_to(out, ".{} = {}",  identifier_of(member), t.[:member:]);
        }

        std::format_to(out, "}}");
        return out;
    }
    
};

struct Foo {
    int a{0};
    struct {
        int iii{242};
    } b;
};

template <> struct std::formatter<decltype(Foo::b)> : UniversalFormatter { };

template <> struct std::formatter<Foo> : UniversalFormatter { };

enum class Color {Green, Red};

template <> struct std::formatter<Color> : EnumFormatter {};

struct Colors {
    int x{0};
    int& y{x};
    void* z{&x};
    Color col{Color::Green};
};

template <> struct std::formatter<Colors> : UniversalFormatter {};

int main() {
    std::println("{}", Foo());
    std::println("{}", Color::Red);
    std::println("{}", Color(242));
    std::println("{}", Colors());
}

/*
Output:

Foo{.a = 0, .b = unnamed{.iii = 242}}
Red
unknown
Colors{.x = 0, .y = 0, .z = 0x7fff0673d390, .col = Green}

 */