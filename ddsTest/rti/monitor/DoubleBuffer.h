#ifndef DOUBLEBUFFER_H
#define DOUBLEBUFFER_H

#include <vector>

//=====================================================================================================================
class AbstractDoubleBuffer
//=====================================================================================================================
{
public:
    static constexpr int RESERVE_SIZE = 2000;
public:
    virtual ~AbstractDoubleBuffer() {}
    virtual void swap() = 0;
    virtual void clear() = 0;
};

//=====================================================================================================================
template<typename T>
class DoubleBuffer : public AbstractDoubleBuffer
//=====================================================================================================================
{

public:
    DoubleBuffer()
        : _iFillBuf(0)
    {
        _buffer[0].reserve(RESERVE_SIZE);
        _buffer[1].reserve(RESERVE_SIZE);
    }

    ~DoubleBuffer() {}

    void swap() final
    {
        _buffer[1-_iFillBuf].clear();
        _iFillBuf = 1 - _iFillBuf;
    }

    void push(T val)
    {
        _buffer[_iFillBuf].push_back(val);
        //std::cout << _buffer[_iFillBuf].size() << std::endl;
    }

    const std::vector<T>& get() const
    {
        return _buffer[1-_iFillBuf];
    }

    void clear() final
    {
        _buffer[0].clear();
        _buffer[1].clear();
    }

private:
    int _iFillBuf;
    std::vector<T> _buffer[2];
};

#endif // DOUBLEBUFFER_H
