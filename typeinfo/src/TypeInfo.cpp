#include "TypeInfo.h"

//---------------------------------------------------------------------------------------------------------------------
std::vector<TypeInfo*>& TypeInfo::singletonDB()
//---------------------------------------------------------------------------------------------------------------------
{
    static std::vector<TypeInfo*> s_typeDatabase;
    return s_typeDatabase;
}

//---------------------------------------------------------------------------------------------------------------------
TypeInfo::TypeInfo()
//---------------------------------------------------------------------------------------------------------------------
    : _pParent(nullptr)
{
}

//---------------------------------------------------------------------------------------------------------------------
TypeInfo::~TypeInfo() = default;
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
bool TypeInfo::isDerivedFromClass(const TypeInfo& baseClassType) const
//---------------------------------------------------------------------------------------------------------------------
{
    const TypeInfo *pTypeInfo = this;

    while (pTypeInfo->_pParent)
    {
        pTypeInfo = pTypeInfo->_pParent;
        if ( pTypeInfo->getClassName() == baseClassType.getClassName() )
        {
            return true;
        }
    }
    return false;
}


//---------------------------------------------------------------------------------------------------------------------
void TypeInfo::listAllTypes (std::ostream &outputStream)
//---------------------------------------------------------------------------------------------------------------------
{
    for(const auto& type : singletonDB())
    {
        if( type->getParentInfo() != nullptr )
        {
            outputStream << type->getClassName() << " (derived from " << type->getParentInfo()->getClassName() << ")" << std::endl;
        }
        else
        {
            outputStream << type->getClassName() << std::endl;
        }
    }
}

