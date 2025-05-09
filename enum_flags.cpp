// How to use enums as flags
// Generated by claude.ai

#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <iostream>
#include <format>
#include <cstdint>
#include <bitset>

// Concept to ensure the template parameter is a scoped enum (enum class)
template <typename E>
concept ScopedEnum = std::is_enum_v<E> && !std::is_convertible_v<E, std::underlying_type_t<E>>;

// Helper struct to add flag operations to any scoped enum
template <ScopedEnum E>
struct Flags {
    using UnderlyingType = std::underlying_type_t<E>;

    E value;

    // Constructors
    constexpr Flags() noexcept : value(static_cast<E>(0)) {}
    constexpr Flags(E value) noexcept : value(value) {}
    constexpr Flags(std::initializer_list<E> flags) noexcept : value(static_cast<E>(0)) {
        for (auto flag : flags) {
            value = static_cast<E>(static_cast<UnderlyingType>(value) |
                                  static_cast<UnderlyingType>(flag));
        }
    }

    // Conversion to underlying type
    constexpr explicit operator UnderlyingType() const noexcept {
        return static_cast<UnderlyingType>(value);
    }

    // Conversion to bool (true if any flags are set)
    constexpr explicit operator bool() const noexcept {
        return static_cast<UnderlyingType>(value) != 0;
    }

    // Check if a flag is set
    constexpr bool has(E flag) const noexcept {
        return (static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flag))
               == static_cast<UnderlyingType>(flag);
    }

    // Check if any of the specified flags are set
    constexpr bool hasAny(Flags flags) const noexcept {
        return (static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flags.value)) != 0;
    }

    // Check if all of the specified flags are set
    constexpr bool hasAll(Flags flags) const noexcept {
        return (static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(flags.value))
               == static_cast<UnderlyingType>(flags.value);
    }

    // Add a flag or flags
    constexpr Flags& set(E flag) noexcept {
        value = static_cast<E>(static_cast<UnderlyingType>(value) |
                              static_cast<UnderlyingType>(flag));
        return *this;
    }

    // Remove a flag or flags
    constexpr Flags& clear(E flag) noexcept {
        value = static_cast<E>(static_cast<UnderlyingType>(value) &
                              ~static_cast<UnderlyingType>(flag));
        return *this;
    }

    // Toggle a flag or flags
    constexpr Flags& toggle(E flag) noexcept {
        value = static_cast<E>(static_cast<UnderlyingType>(value) ^
                              static_cast<UnderlyingType>(flag));
        return *this;
    }

    // Reset all flags
    constexpr Flags& reset() noexcept {
        value = static_cast<E>(0);
        return *this;
    }

    // Bitwise operations
    constexpr Flags operator|(E flag) const noexcept {
        return Flags(static_cast<E>(static_cast<UnderlyingType>(value) |
                                   static_cast<UnderlyingType>(flag)));
    }

    constexpr Flags operator&(E flag) const noexcept {
        return Flags(static_cast<E>(static_cast<UnderlyingType>(value) &
                                   static_cast<UnderlyingType>(flag)));
    }

    constexpr Flags operator^(E flag) const noexcept {
        return Flags(static_cast<E>(static_cast<UnderlyingType>(value) ^
                                   static_cast<UnderlyingType>(flag)));
    }

    constexpr Flags operator~() const noexcept {
        return Flags(static_cast<E>(~static_cast<UnderlyingType>(value)));
    }

    constexpr Flags& operator|=(E flag) noexcept {
        value = static_cast<E>(static_cast<UnderlyingType>(value) |
                              static_cast<UnderlyingType>(flag));
        return *this;
    }

    constexpr Flags& operator&=(E flag) noexcept {
        value = static_cast<E>(static_cast<UnderlyingType>(value) &
                              static_cast<UnderlyingType>(flag));
        return *this;
    }

    constexpr Flags& operator^=(E flag) noexcept {
        value = static_cast<E>(static_cast<UnderlyingType>(value) ^
                              static_cast<UnderlyingType>(flag));
        return *this;
    }

    // Comparison operators
    constexpr bool operator==(const Flags&) const = default;

    // Return the number of flags set
    constexpr std::size_t count() const noexcept {
        return std::bitset<sizeof(UnderlyingType) * 8>(static_cast<UnderlyingType>(value)).count();
    }
};

// Operators for combining enum values directly
template <ScopedEnum E>
constexpr Flags<E> operator|(E lhs, E rhs) noexcept {
    using UT = std::underlying_type_t<E>;
    return Flags<E>(static_cast<E>(static_cast<UT>(lhs) | static_cast<UT>(rhs)));
}

template <ScopedEnum E>
constexpr Flags<E> operator&(E lhs, E rhs) noexcept {
    using UT = std::underlying_type_t<E>;
    return Flags<E>(static_cast<E>(static_cast<UT>(lhs) & static_cast<UT>(rhs)));
}

template <ScopedEnum E>
constexpr Flags<E> operator^(E lhs, E rhs) noexcept {
    using UT = std::underlying_type_t<E>;
    return Flags<E>(static_cast<E>(static_cast<UT>(lhs) ^ static_cast<UT>(rhs)));
}

template <ScopedEnum E>
constexpr Flags<E> operator~(E rhs) noexcept {
    using UT = std::underlying_type_t<E>;
    return Flags<E>(static_cast<E>(~static_cast<UT>(rhs)));
}

// User-defined literal for creating a Flags object from an enum value
//template <ScopedEnum E>
//constexpr Flags<E> operator""_flag(E value) noexcept {
//    return Flags<E>(value);
//}

// NTTP (Non-Type Template Parameter) helper to define flag values
template <std::size_t N>
constexpr auto flag_value = 1ULL << N;

// Flag reflection utilities (requires C++23 for string literals in templates)
template <ScopedEnum E>
struct FlagNames {};

// Helper for converting flag values to strings
template <ScopedEnum E>
std::string to_string(Flags<E> flags) {
    if (static_cast<std::underlying_type_t<E>>(flags.value) == 0) {
        return "None";
    }

    std::string result;
    bool first = true;

    // This part needs specialization for each enum type
    // Here we provide a generic implementation
    for (int i = 0; i < sizeof(std::underlying_type_t<E>) * 8; ++i) {
        auto flag_val = static_cast<E>(1ULL << i);
        if (flags.has(flag_val)) {
            if (!first) {
                result += " | ";
            }
            // Try to use specialized flag names if available
            if constexpr (requires { FlagNames<E>::get(flag_val); }) {
                result += FlagNames<E>::get(flag_val);
            } else {
                result += std::format("Flag({})", i);
            }
            first = false;
        }
    }

    return result;
}

// Macro to simplify the creation of flag name specializations
#define DEFINE_FLAG_NAMES(EnumType, ...) \
template <> \
struct FlagNames<EnumType> { \
    static std::string_view get(EnumType flag) { \
        using UT = std::underlying_type_t<EnumType>; \
        switch (flag) { \
            __VA_ARGS__ \
            default: return "Unknown"; \
        } \
    } \
};

// Example usage:
enum class LoggingFlag : std::uint32_t {
    None     = 0,
    Info     = flag_value<0>,
    Warning  = flag_value<1>,
    Error    = flag_value<2>,
    Debug    = flag_value<3>,
    Trace    = flag_value<4>
};

// Define names for our flags
DEFINE_FLAG_NAMES(LoggingFlag,
    case LoggingFlag::None: return "None";
    case LoggingFlag::Info: return "Info";
    case LoggingFlag::Warning: return "Warning";
    case LoggingFlag::Error: return "Error";
    case LoggingFlag::Debug: return "Debug";
    case LoggingFlag::Trace: return "Trace";
)

enum class SubsystemFlag : std::uint16_t {
    None     = 0,
    Logger   = flag_value<0>,
    Ipc      = flag_value<1>,
    Signals  = flag_value<2>,
    Network  = flag_value<3>
};

DEFINE_FLAG_NAMES(SubsystemFlag,
    case SubsystemFlag::None: return "None";
    case SubsystemFlag::Logger: return "Logger";
    case SubsystemFlag::Ipc: return "Ipc";
    case SubsystemFlag::Signals: return "Signals";
    case SubsystemFlag::Network: return "Network";
)

class Application {
private:
    Flags<SubsystemFlag> m_enabledSubsystems;

public:
    Application() = default;

    void EnableSubsystem(SubsystemFlag subsystem) {
        m_enabledSubsystems.set(subsystem);
        std::cout << "Enabled: " << to_string(Flags<SubsystemFlag>(subsystem)) << std::endl;
    }

    void EnableSubsystems(Flags<SubsystemFlag> subsystems) {
        m_enabledSubsystems |= subsystems.value;
        std::cout << "Enabled: " << to_string(subsystems) << std::endl;
    }

    void DisableSubsystem(SubsystemFlag subsystem) {
        m_enabledSubsystems.clear(subsystem);
        std::cout << "Disabled: " << to_string(Flags<SubsystemFlag>(subsystem)) << std::endl;
    }

    bool IsSubsystemEnabled(SubsystemFlag subsystem) const {
        return m_enabledSubsystems.has(subsystem);
    }

    void PrintEnabledSubsystems() const {
        std::cout << "Currently enabled: " << to_string(m_enabledSubsystems) << std::endl;
    }
};

class Logger {
private:
    Flags<LoggingFlag> m_enabledLevels;

public:
    Logger() : m_enabledLevels(LoggingFlag::Info | LoggingFlag::Warning | LoggingFlag::Error) {}

    void SetLevel(LoggingFlag level, bool enabled) {
        if (enabled) {
            m_enabledLevels.set(level);
            std::cout << "Enabled log level: " << to_string(Flags<LoggingFlag>(level)) << std::endl;
        } else {
            m_enabledLevels.clear(level);
            std::cout << "Disabled log level: " << to_string(Flags<LoggingFlag>(level)) << std::endl;
        }
    }

    void Log(LoggingFlag level, std::string_view message) {
        if (m_enabledLevels.has(level)) {
            std::cout << "[" << to_string(Flags<LoggingFlag>(level)) << "] " << message << std::endl;
        }
    }

    void PrintEnabledLevels() const {
        std::cout << "Enabled log levels: " << to_string(m_enabledLevels) << std::endl;
    }
};

int main() {
    // Application example
    std::cout << "=== Application Example ===" << std::endl;
    Application app;

    std::cout << "Initializing application..." << std::endl;
    app.PrintEnabledSubsystems();

    std::cout << "\nEnabling Logger..." << std::endl;
    app.EnableSubsystem(SubsystemFlag::Logger);
    app.PrintEnabledSubsystems();

    std::cout << "\nEnabling IPC and Signals..." << std::endl;
    app.EnableSubsystems(SubsystemFlag::Ipc | SubsystemFlag::Signals);
    app.PrintEnabledSubsystems();

    std::cout << "\nIs Logger enabled? " <<
        (app.IsSubsystemEnabled(SubsystemFlag::Logger) ? "Yes" : "No") << std::endl;

    std::cout << "\nDisabling IPC..." << std::endl;
    app.DisableSubsystem(SubsystemFlag::Ipc);
    app.PrintEnabledSubsystems();

    // Logger example
    std::cout << "\n\n=== Logger Example ===" << std::endl;
    Logger logger;

    std::cout << "Logger initialized with default levels:" << std::endl;
    logger.PrintEnabledLevels();

    logger.Log(LoggingFlag::Info, "This is an info message");
    logger.Log(LoggingFlag::Debug, "This debug message won't be displayed");

    std::cout << "\nEnabling Debug level..." << std::endl;
    logger.SetLevel(LoggingFlag::Debug, true);
    logger.PrintEnabledLevels();

    logger.Log(LoggingFlag::Debug, "Now this debug message will be displayed");

    // Using more advanced features
    std::cout << "\n\n=== Advanced Features ===" << std::endl;

    auto subsystems = Flags<SubsystemFlag>{SubsystemFlag::Logger, SubsystemFlag::Network};
    std::cout << "Created flags from initializer list: " << to_string(subsystems) << std::endl;

    auto logging_flags = LoggingFlag::Info | LoggingFlag::Warning;
    std::cout << "Combined flags: " << to_string(logging_flags) << std::endl;

    std::cout << "Number of flags set: " << logging_flags.count() << std::endl;

    auto toggled = logging_flags;
    toggled.toggle(LoggingFlag::Info);
    toggled.toggle(LoggingFlag::Debug);
    std::cout << "After toggling Info and Debug: " << to_string(toggled) << std::endl;

    return 0;
}
