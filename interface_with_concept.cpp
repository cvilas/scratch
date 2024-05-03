#include <vector>
#include <array>
#include <concepts>
#include <string>

// Demonstrates implementing interfaces with C++20 concept
// https://www.reachablecode.com/2019/06/09/concept-based-interfaces/

// Defines a signal processor interface
template<typename SignalProcessorType, typename DataType>
concept SignalProcessor = requires(SignalProcessorType p, DataType d) {
    { p.process(d) } -> std::same_as<DataType>;
    { p.reset() };
};

// Defines a concrete class that satisfies the interface
template<typename DataType>
struct LowPassFilter{
    LowPassFilter() requires(SignalProcessor<LowPassFilter<DataType>,DataType>) {}
    auto process(const DataType& d) -> DataType;
    void reset();
};

struct Data{
    float v[3];
};

LowPassFilter<std::vector<int>> ia;
LowPassFilter<std::array<double,3>> da;
LowPassFilter<double> d;
LowPassFilter<Data> sa;
LowPassFilter<std::string> ss;
