#include "EmbeddedSystem.h"
#include "PlantMessenger.h"
#include <math.h>
#include <chrono>
#include <iostream>
#include <thread>

//---------------------------------------------------------------------------------------------------------------------
EmbeddedSystem::EmbeddedSystem(int domainId)
//---------------------------------------------------------------------------------------------------------------------
    : _ticks(0),
      _output(0),
      _amplitude(MIN_AMPLITUDE),
      _frequencyHz(MIN_FREQUENCY_HZ),
      _messenger(domainId)
{
    // set up messenger
    _messenger.registerControlVariable("amplitude", &_amplitude, MIN_AMPLITUDE, MAX_AMPLITUDE);
    _messenger.registerControlVariable("frequencyHz", &_frequencyHz, MIN_FREQUENCY_HZ, MAX_FREQUENCY_HZ);
    _messenger.registerLogVariable("output", &_output);

    // start process
    _exitFlag = false;
    _thread = std::thread( [this] { process(); });
}

//---------------------------------------------------------------------------------------------------------------------
EmbeddedSystem::~EmbeddedSystem()
//---------------------------------------------------------------------------------------------------------------------
{
    _exitFlag = true;
    _thread.join();
}

//---------------------------------------------------------------------------------------------------------------------
void EmbeddedSystem::process()
//---------------------------------------------------------------------------------------------------------------------
{
    static const auto t0 = std::chrono::high_resolution_clock::now();

    while(!_exitFlag)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        // obtain control parameters from remote
        _messenger.updateAllControlVariables();

        // Do the process step
        auto t = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> seconds = t-t0;
        _output = _amplitude * sin(2 * M_PI * _frequencyHz * seconds.count());
        _ticks++;
        //std::cout << _output << " " << seconds.count() << std::endl;

        // update remote with results
        _messenger.transmitAllLogVariables();

    }
}
