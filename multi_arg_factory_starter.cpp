/*
#include <memory>
#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>

struct BaseClass {
    virtual ~BaseClass() = default;
    virtual std::unique_ptr<BaseClass> create() = 0;

};


template<typename... Ts>
using Creator = std::function<std::unique_ptr<BaseClass>(Ts&&...params)>;

std::unordered_map<std::string, Creator> typedb;

struct MyClass : BaseClass {

    int value;

    MyClass(int v) : value(v){}

    template<typename... Ts>
    std::unique_ptr<BaseClass> create(Ts&&... params) { 
        if constexpr (std::is_constructible_v<MyClass, Ts...>)
            return std::make_unique<MyClass>(std::forward<Ts>(params)...);
        else
            return nullptr;
    }
};

struct MyOtherClass : BaseClass {

    std::string str;

    MyOtherClass(std::string s) : str(s){}

    template<typename... Ts>
    std::unique_ptr<BaseClass> create(Ts&&... params) { 
        if constexpr (std::is_constructible_v<MyOtherClass, Ts...>)
            return std::make_unique<MyOtherClass>(std::forward<Ts>(params)...);
        else
            return nullptr;
    }
};

int main(int argc, const char* argv[]) {
    return 0;
}
*/

#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>

struct Base{
    virtual ~Base() = default;
};

struct Derived : Base{
    int i_;
    Derived(int i) : Base(), i_(i){}
    static std::unique_ptr<Base> create(int i) {
        std::cout << "Dervied::create" << "\n";
        return std::make_unique<Derived>(i);
    }
};

template<typename T>
struct OtherDerived : Base{
    T value_;
    OtherDerived(T s) : Base(), value_(s){}
    static std::unique_ptr<Base> create(T s) {
        std::cout << "OtherDerived::create" << "\n";
        return std::make_unique<OtherDerived>(s);
    }
};


// Base class for all functions so that we can store all functions in a single container.
struct Function {
  virtual ~Function() { }
};

// Derived class template for functions with a particular signature.
template <typename T>
struct BasicFunction : Function {
  std::function<T> function;
  BasicFunction(std::function<T> function) : function(function) { }
};

// Generic container of creators
typedef std::unordered_map<std::string,std::unique_ptr<Function> > CreatorMap;

template <typename Func>
static void addCreator(CreatorMap &map, std::string name, Func &function)
{
  std::unique_ptr<Function> func_ptr(new BasicFunction<Func>(std::function<Func>(function)));
  map.insert(CreatorMap::value_type(name,std::move(func_ptr)));
}

template<class... Args>
std::unique_ptr<Base> create(CreatorMap& map, std::string name, Args&&... args) {
  typedef std::unique_ptr<Base> Func(typename std::remove_reference<Args>::type...);
  const Function &f = *map.at(name);
  std::function<Func> func = static_cast<const BasicFunction<Func> &>(f).function;
  return func(std::forward<Args>(args)...);
}

int main (int argc, char **argv) {
  CreatorMap m;
  addCreator(m, "Derived", Derived::create);
  addCreator(m, "OtherDerived<D>", OtherDerived<double>::create);
  addCreator(m, "OtherDerived<string>", OtherDerived<std::string>::create);

  const auto a = create(m, "Derived", 2);
  std::cout << dynamic_cast<Derived*>(a.get())->i_ << "\n";

  const auto b = create(m, "OtherDerived<D>", 4.5);
  std::cout << dynamic_cast<OtherDerived<double>*>(b.get())->value_ << "\n";

  const auto c = create(m, "OtherDerived<string>", "vilas");
  std::cout << dynamic_cast<OtherDerived<std::string>*>(c.get())->value_ << "\n";

  return 0;
}
