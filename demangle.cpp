#include <iostream>
#include <memory>
#include <cxxabi.h>

#if defined(__clang__)
static const char personality[] = "Clang/LLVM";
#elif defined(__ICC) || defined(__INTEL_COMPILER)
static const char personality[] = "Intel ICC/ICPC";
#elif defined(__GNUC__) || defined(__GNUG__)
static const char personality[] = "GNU GCC/G++";
#elif defined(_MSC_VER)
static const char personality[] = "Microsoft Visual Studio";
#else
static const char personality[] = "Unknown";
#endif

//---------------------------------------------------------------------------------------------------------------------
std::string demangle(const char* mangled_name)
//---------------------------------------------------------------------------------------------------------------------
{
  /// \note reference:
  /// https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname

  int status = -4;  // some arbitrary value to eliminate the compiler warning

  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void (*)(void*)> res{ abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), std::free };

  return (status == 0) ? res.get() : mangled_name;
}

class TestClass
{};

int main()
{
    std::cout << "Using " << personality << " " << demangle(typeid(TestClass).name()) << std::endl;
    return 0;
}
