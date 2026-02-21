//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include <iio.h>

namespace sense_hat::iio {

//=================================================================================================
struct HTS221Data {
    double humidity_percent{ 0.0 };
    double temperature_celsius{ 0.0 };
    std::chrono::system_clock::time_point timestamp{};
};

//=================================================================================================
class HTS221Sensor {
public:
    struct Config {
        std::string device_name{ "hts221" };
    };

    explicit HTS221Sensor(const Config& config);

    [[nodiscard]] auto readSample(HTS221Data& data) const -> bool;
    [[nodiscard]] auto deviceInfo() const -> std::string;
    [[nodiscard]] auto availableSamplingFrequencies() const -> std::string;
    [[nodiscard]] auto setSamplingFrequency(int frequency_hz) const -> bool;

private:
    struct ContextDeleter {
        void operator()(iio_context* context) const;
    };

    using ContextPtr = std::unique_ptr<iio_context, ContextDeleter>;

    [[nodiscard]] auto requireDevice(const std::string& device_name) const -> iio_device*;
    [[nodiscard]] auto requireChannel(const char* channel_id) const -> iio_channel*;

    static auto readChannelAttrDouble(iio_channel* channel, const char* attribute, double& value)
            -> bool;
    static auto
    readChannelAttrLongLong(iio_channel* channel, const char* attribute, long long& value) -> bool;

    ContextPtr context_{ nullptr };
    iio_device* device_{ nullptr };
    iio_channel* humidity_channel_{ nullptr };
    iio_channel* temperature_channel_{ nullptr };
};

}  // namespace sense_hat::iio