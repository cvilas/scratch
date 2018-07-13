#include "TypeInfoT.h"
#include <iostream>

///====================================================================================================================
class Base
{
public:
    Base() {}
    virtual ~Base() {}
    static TypeInfoT<Base> s_typeInfo;
    virtual const TypeInfo *getTypeInfo() const { return &s_typeInfo; }
    virtual void message() { std::cout << "Base" << std::endl; }
};

TypeInfoT<Base> Base::s_typeInfo("");


///====================================================================================================================
class Derived : public Base
{
public:
    Derived() : Base() {}
    virtual ~Derived() {}
    static TypeInfoT<Derived> s_typeInfo;
    virtual const TypeInfo *getTypeInfo() const { return &s_typeInfo; }
    virtual void message() { std::cout << "Derived" << std::endl; }
};

TypeInfoT<Derived> Derived::s_typeInfo("Base");

///====================================================================================================================
int main(int argc, char *argv[])
///====================================================================================================================
{
    Base b;
    Derived d;

    TypeInfo::listAllTypes(std::cout);

    return 0;
}
