#include <functional>
#include <print>
#include <array>

// A function that triggers on rising and falling edge rather than state. Useful in logging state changes within loops
inline constexpr void on_edge(bool state, const std::invocable<> auto& on_rising, const std::invocable<> auto& on_falling) {
    [&]{
        static bool prev_state = state;
        if (!prev_state && state) {
            on_rising();
        } else if (prev_state && !state) {
            on_falling();
        }
        prev_state = state;
    }();
}

int main() {
    constexpr auto condA = std::array{false, true, true, false, true};
    constexpr auto condB = std::array{true, true, false, false, true};

    for (size_t i = 0; i < 5; ++i) {
        on_edge(condA[i], []{ std::println("A Entry"); }, []{ std::println("A Exit"); });
        on_edge(condB[i], []{ std::println("B Entry"); }, []{ std::println("B Exit"); });
    }

}