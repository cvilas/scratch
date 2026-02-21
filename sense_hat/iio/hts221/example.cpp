//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "hts221.h"

#include <array>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <thread>

namespace {

using sense_hat::iio::HTS221Data;
using sense_hat::iio::HTS221Sensor;

//-------------------------------------------------------------------------------------------------
auto exampleSingleRead() -> void {
    auto sensor = HTS221Sensor({});
    auto sample = HTS221Data{};

    if (!sensor.readSample(sample)) {
        std::cout << "Failed to read sensor" << std::endl;
        return;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Humidity: " << sample.humidity_percent << " % RH" << std::endl;
    std::cout << "Temperature: " << sample.temperature_celsius << " C" << std::endl;
}

//-------------------------------------------------------------------------------------------------
auto exampleSamplingRate() -> void {
    auto sensor = HTS221Sensor({});
    std::cout << "Available frequencies: " << sensor.availableSamplingFrequencies() << std::endl;

    constexpr auto rates = std::array<int, 3>{ 1, 7, 12 };
    for (const auto rate : rates) {
        std::cout << "Setting rate to " << rate << " Hz" << std::endl;
        if (!sensor.setSamplingFrequency(rate)) {
            std::cout << "Could not set sampling frequency" << std::endl;
            continue;
        }

        for (auto index = 0; index < 3; ++index) {
            auto sample = HTS221Data{};
            if (sensor.readSample(sample)) {
                std::cout << std::fixed << std::setprecision(2);
                std::cout << sample.humidity_percent << " % RH, " << sample.temperature_celsius << " C"
                                    << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}

}  // namespace

//=================================================================================================
auto main(int argc, char* argv[]) -> int {
    try {
        std::string mode = "all";
        if (argc > 1) {
            mode = argv[1];
        }

        if ((mode == "single") || (mode == "all")) {
            exampleSingleRead();
        }
        if ((mode == "rate") || (mode == "all")) {
            exampleSamplingRate();
        }

        if ((mode != "single") && (mode != "rate") && (mode != "all")) {
            std::cout << "Usage: " << argv[0] << " [single|rate|all]" << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::puts(ex.what());
        return EXIT_FAILURE;
    }
}