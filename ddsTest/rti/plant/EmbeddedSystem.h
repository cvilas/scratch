#ifndef EMBEDDEDSYSTEM_H
#define EMBEDDEDSYSTEM_H

#include <thread>
#include <atomic>
#include "PlantMessenger.h"

/// An example plant that does some control action in the form of sine generator
/// Remotely settable control parameters: frequency, amplitude
/// Remotely loggable state: sine wave output
class EmbeddedSystem
{
public:
    static constexpr double MIN_AMPLITUDE = 0.1;
    static constexpr double MAX_AMPLITUDE = 100;

    static constexpr double MIN_FREQUENCY_HZ = 1;
    static constexpr double MAX_FREQUENCY_HZ = 10;

public:
    EmbeddedSystem(int domainId);
    virtual ~EmbeddedSystem();

private:
    void process();

private:
    long int            _ticks;
    double              _output;
    double              _amplitude;
    double              _frequencyHz;
    std::atomic<bool>   _exitFlag;
    std::thread         _thread;

    PlantMessenger  _messenger;
};

#endif // EMBEDDEDSYSTEM_H
