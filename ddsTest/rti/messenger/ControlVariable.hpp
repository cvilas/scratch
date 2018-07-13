#ifndef CONTROLVARIABLE_HPP
#define CONTROLVARIABLE_HPP

#include <stddef.h>

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void ControlVariableT<T>::fromBytes(const std::vector<uint8_t>& bytes)
//---------------------------------------------------------------------------------------------------------------------
{
    // serialiased as [value][min][max]

    const T* v = reinterpret_cast<const T*>(&bytes[0]);
    _value = v[0];
    _min = v[1];
    _max = v[2];
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::vector<uint8_t> ControlVariableT<T>::toBytes() const
//---------------------------------------------------------------------------------------------------------------------
{
    // serialiased as [value][min][max]
    size_t len = sizeof(T);
    std::vector<uint8_t> bytes;
    bytes.reserve(3 * len);

    const uint8_t* v = reinterpret_cast<const uint8_t*>(&_value);
    for(size_t i = 0; i < len; ++i)
    {
        bytes.push_back(v[i]);
    }

    v = reinterpret_cast<const uint8_t*>(&_min);
    for(size_t i = 0; i < len; ++i)
    {
        bytes.push_back(v[i]);
    }

    v = reinterpret_cast<const uint8_t*>(&_max);
    for(size_t i = 0; i < len; ++i)
    {
        bytes.push_back(v[i]);
    }
    return bytes;
}

#endif // CONTROLVARIABLE_HPP
