#pragma once

#include "TypeInfo.h"
#include <cstdlib>
#include <memory>

#include <cxxabi.h>

///====================================================================================================================
/// Template class to store type information
///====================================================================================================================
template<typename T>
class TypeInfoT : public TypeInfo
{
public:
    TypeInfoT(const std::string& getParentClassName="");
    ~TypeInfoT() {}
    std::string getClassName() const final { return _className; }
    std::string getParentClassName() const final { return _parentClassName;}
    void* createObject() const { return new T; }
private:
    void registerType();
    std::string _className;
    std::string _parentClassName;
};


//---------------------------------------------------------------------------------------------------------------------
std::string demangle(const char* mangled_name)
//---------------------------------------------------------------------------------------------------------------------
{
  int status = -4;  // some arbitrary value to eliminate the compiler warning
  std::unique_ptr<char, void (*)(void*)> res{ abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), std::free };
  return (status == 0) ? res.get() : mangled_name;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
TypeInfoT<T>::TypeInfoT(const std::string& parentClassName)
//---------------------------------------------------------------------------------------------------------------------
    : TypeInfo(), _className(demangle(typeid(T).name())), _parentClassName(parentClassName)
{    
    registerType();
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void TypeInfoT<T>::registerType()
//---------------------------------------------------------------------------------------------------------------------
{
    std::vector<TypeInfo*>& db = singletonDB();

    db.push_back(this);
    std::cout << "[TypeInfo] Registering type " << _className << " (DB size: " << db.size()<< ")"<< std::endl;

    for(auto& t : db )
    {
        // find the parent
        if( t->getClassName() == getParentClassName() )
        {
            _pParent = t;
        }

        // are we a parent
        if( getClassName() == t->getParentClassName() )
        {
            ((TypeInfoT<T>*)t)->_pParent = this;
        }
    }
}
