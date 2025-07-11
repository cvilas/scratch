#include <iostream>
#include <type_traits>

// a type
struct Key
{
  char bytes[16];
};

// a class that implements a method that returns the above type (\todo)
class MyType
{
public:
  Key getKey() const { }
};

template<typename T>
struct has_getKey_method
{
private:
	using yes = std::true_type;
	using no = std::false_type;
 
  // Use C++ template SFINAE (Substitution Failure Is Not An Error) feature
  // Use first form of test if substitution U.getKey succeeds, else use the second one which is a catch-all sinkhole
  template<typename U> static auto test(int/*unused*/) -> decltype(std::declval<U>().getKey(), yes());
  template<typename> static no test(...);
 
public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),yes>::value;
};

// usage
int main()
{
  std::cout << std::boolalpha;
  std::cout << has_getKey_method<int>::value << std::endl;
  std::cout << has_getKey_method<MyType>::value << std::endl;
}
