#ifndef CONTROLVARIABLE_H
#define CONTROLVARIABLE_H

#include <vector>
#include <stdint.h>

///=====================================================================================================================
/// Abstract interface to control parameters
///=====================================================================================================================
class AbstractControlVariable
{
public:
    virtual ~AbstractControlVariable() = default;
    virtual void fromBytes(const std::vector<uint8_t>& bytes) = 0;
    virtual std::vector<uint8_t> toBytes() const = 0;
};

///=====================================================================================================================
/// Template interface to control parameters
///=====================================================================================================================
template <typename T>
class ControlVariableT : public AbstractControlVariable
{
public:

    ControlVariableT() : ControlVariableT(0,0,0) {}

    ControlVariableT(const T& value, const T& min, const T& max): _value(value), _min(min), _max(max) {}

    ~ControlVariableT() {}

    void value(const T& value)
    {
        if(value < _min)        _value = _min;
        else if(value > _max)   _value = _max;
        else                    _value = value;

    }

    const T& value() const { return _value; }

    const T& min() const { return _min; }

    const T& max() const { return _max; }

    void fromBytes(const std::vector<uint8_t>& bytes) final;

    std::vector<uint8_t> toBytes() const final;

private:
    T _value;
    T _min;
    T _max;
};

#include "ControlVariable.hpp"

#endif // CONTROLVARIABLE_H
