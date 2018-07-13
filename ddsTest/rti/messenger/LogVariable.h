#ifndef LOGVARIABLE_H
#define LOGVARIABLE_H

#include <vector>

///=====================================================================================================================
/// Abstract interface to log variables
///=====================================================================================================================
class AbstractLogVariable
{
public:
    virtual ~AbstractLogVariable() = default;
    virtual void fromBytes(const std::vector<uint8_t>& bytes) = 0;
    virtual std::vector<uint8_t> toBytes() const = 0;

    virtual void serialise(std::ostream& stream) const = 0;
    virtual void deserialise(std::istream& stream) = 0;
};

///=====================================================================================================================
/// Template interface to log variables
///=====================================================================================================================
template<typename T>
class LogVariableT : public AbstractLogVariable
{
public:
    LogVariableT() : _value(0) {}
    LogVariableT(const T& value) : _value(value) {}
    ~LogVariableT() {}
    const T& value() const { return _value; }
    void fromBytes(const std::vector<uint8_t>& bytes) final;
    std::vector<uint8_t> toBytes() const final;
    void serialise(std::ostream& stream) const final;
    void deserialise(std::istream& stream) final;
private:
    T _value;
};

#include "LogVariable.hpp"

#endif // LOGVARIABLE_H
