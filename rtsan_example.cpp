// llvm function effect analysis and realtime sanitizer
// Requires clang version 20 and above 
// clang++ -DENABLE_FEA -fsanitize=realtime -o rtsan_example rtsan_example

#include <cstdlib>

#ifdef ENABLE_FEA
    #define GRAPE_NONBLOCKING [[clang::nonblocking]]
    #define GRAPE_FEA_IGNORE(...) \
      _Pragma("clang diagnostic push") \
      _Pragma("clang diagnostic ignored \"-Wfunction-effects\"") \
      __VA_ARGS__ _Pragma("clang diagnostic pop")
#else
    #define GRAPE_NONBLOCKING
    #define GRAPE_FEA_IGNORE(...) __VA_ARGS__
#endif

void process() GRAPE_NONBLOCKING {
    auto ptr = new int(2);
}

int main() {
    process();
    return EXIT_SUCCESS;
}