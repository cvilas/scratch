//g++  4.9.3
#include <iostream>

template <typename T>
void print(T const &elem)
{
    std::cout << elem << std::endl;
}

template <typename T, typename... Args>
void print(T const &elem, Args&&... pack)
{
    std::cout << elem << std::endl;
    print(pack...);
}

template <typename FuncT>
struct ABase;

template <typename ReturnT, typename... Args>
struct ABase<ReturnT(Args...)>
{
    virtual ReturnT get_any(Args&&...) = 0;
    virtual ~ABase() = default;
};

template <typename FuncT>
struct ADerived;

template <typename ReturnT, typename... Args>
struct ADerived<ReturnT(Args...)> : ABase<ReturnT(Args...)>
{
    ReturnT get_any(Args&&... pack) override
    {
        std::cout << "Parameters which passed to virtual "
                    << "function with non-fixed number of parameters:\n";
        print(pack...);
    }
};

int main(int argc, char **argv)
{
    ADerived<void(int, int, int, int, int, int, int, int, int, int)> ad;
    ad.get_any(1,2,3,4,5,6,7,8,9,10);

    return 0;
}
