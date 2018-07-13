#ifndef LOGVARIABLE_HPP
#define LOGVARIABLE_HPP

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::vector<uint8_t> LogVariableT<T>::toBytes() const
//---------------------------------------------------------------------------------------------------------------------
{
    // serialiased as [value]
    size_t len = sizeof(T);
    std::vector<uint8_t> bytes(len);

    const uint8_t* v = reinterpret_cast<const uint8_t*>(&_value);
    memcpy(&bytes[0], v, len);
    return bytes;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void LogVariableT<T>::fromBytes(const std::vector<uint8_t>& bytes)
//---------------------------------------------------------------------------------------------------------------------
{
    // serialiased as [value]

    const T* v = reinterpret_cast<const T*>(&bytes[0]);
    _value = v[0];
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void LogVariableT<T>:: serialise(std::ostream& stream) const
//---------------------------------------------------------------------------------------------------------------------
{
    stream << _value;
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void LogVariableT<T>:: deserialise(std::istream& stream)
//---------------------------------------------------------------------------------------------------------------------
{
    stream >> _value;
}


#endif // LOGVARIABLE_HPP
