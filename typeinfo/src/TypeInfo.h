#pragma once

#include <string>
#include <memory>
#include <vector>
#include <iostream>

///====================================================================================================================
/// Base class to store run-time type information.
///
/// The classes TypeInfo and TypeInfoT store run-time type information (i.e., class name and parent class) and also
/// maintain a static type database (a list of pointers to class TypeInfo). While the type database is built, the
/// inheritance structure is also stored with it. Note: Multiple inheritance is not supported.
///
/// The procedure to build a type database is as follows:
/// <ol>
/// <li> Each class has a static member \a s_typeInfo of class TypeInfoT
/// <li> At program start, all these static members are constructed.
/// <li> Each time a TypeInfoT object is constructed, it appends itself to the list of types
/// <li> At the point that \a main() is called, the type database is complete
/// </ol>
///
/// To use the type information, either
/// <ul>
/// <li> use the \a s_typeInfo data member of an object to determine its type
/// <li> use the type database to retrieve a list of all types (static functions of the TypeInfo class)
/// </ul>
///
/// <b>Example Program:</b>
/// \include TypeInfoTest.cpp
///====================================================================================================================
class TypeInfo
{
public:
    TypeInfo();
    virtual ~TypeInfo();
    static size_t numTypesRegistered() { return singletonDB().size(); }
    static void listAllTypes(std::ostream &outputStream = std::cout);
    virtual std::string getClassName() const = 0;
    virtual std::string getParentClassName() const = 0;
    const TypeInfo* getParentInfo() const { return _pParent; }
    bool isClass(const std::string& className) const { return className == getClassName(); }
    bool isDerivedFromClass(const TypeInfo& baseClassType) const;
    virtual void* createObject() const = 0;

protected:
    static std::vector<TypeInfo*>& singletonDB();
    virtual void registerType() = 0;

protected:
    TypeInfo*   _pParent;
};
