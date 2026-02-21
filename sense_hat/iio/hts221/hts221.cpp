//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "hts221.h"

#include <array>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace sense_hat::iio {

namespace {
//-------------------------------------------------------------------------------------------------
auto trimTrailingNewline(std::string* value) -> void {
    if (value->empty()) {
        return;
    }

    if (value->back() == '\n') {
        value->pop_back();
    }
}
}  // namespace

//-------------------------------------------------------------------------------------------------
void HTS221Sensor::ContextDeleter::operator()(iio_context* context) const {
    if (context != nullptr) {
        iio_context_destroy(context);
    }
}

//-------------------------------------------------------------------------------------------------
HTS221Sensor::HTS221Sensor(const Config& config) : context_(iio_create_local_context()) {
    if (context_ == nullptr) {
        throw std::runtime_error("Failed to create local IIO context");
    }

    device_ = requireDevice(config.device_name);
    humidity_channel_ = requireChannel("humidityrelative");
    temperature_channel_ = requireChannel("temp");
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::requireDevice(const std::string& device_name) const -> iio_device* {
    iio_device* device = iio_context_find_device(context_.get(), device_name.c_str());
    if (device != nullptr) {
        return device;
    }

    std::ostringstream stream;
    stream << "IIO device '" << device_name << "' not found. Available devices:";

    const auto count = iio_context_get_devices_count(context_.get());
    for (unsigned int index = 0; index < count; ++index) {
        auto* current = iio_context_get_device(context_.get(), index);
        const char* name = iio_device_get_name(current);
        const char* id = iio_device_get_id(current);
        stream << "\n- " << (id != nullptr ? id : "unknown") << " ("
                     << (name != nullptr ? name : "unnamed") << ")";
    }

    throw std::runtime_error(stream.str());
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::requireChannel(const char* channel_id) const -> iio_channel* {
    auto* channel = iio_device_find_channel(device_, channel_id, false);
    if (channel == nullptr) {
        std::ostringstream stream;
        stream << "Channel not found: " << channel_id;
        throw std::runtime_error(stream.str());
    }

    return channel;
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::readChannelAttrDouble(iio_channel* channel, const char* attribute,
                                                                                 double& value) -> bool {
    const auto ret = iio_channel_attr_read_double(channel, attribute, &value);
    return (ret == 0);
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::readChannelAttrLongLong(iio_channel* channel, const char* attribute,
                                                                                     long long& value) -> bool {
    const auto ret = iio_channel_attr_read_longlong(channel, attribute, &value);
    return (ret == 0);
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::deviceInfo() const -> std::string {
    std::ostringstream stream;

    const char* const device_name = iio_device_get_name(device_);
    const char* const device_id = iio_device_get_id(device_);
    stream << "Device: " << (device_name != nullptr ? device_name : "unnamed") << '\n';
    stream << "ID: " << (device_id != nullptr ? device_id : "unknown") << '\n';

    const auto channels_count = iio_device_get_channels_count(device_);
    stream << "Channels: " << channels_count << '\n';

    for (unsigned int index = 0; index < channels_count; ++index) {
        auto* channel = iio_device_get_channel(device_, index);
        const char* id = iio_channel_get_id(channel);
        const char* name = iio_channel_get_name(channel);

        stream << "  [" << index << "] " << (id != nullptr ? id : "unknown");
        if (name != nullptr) {
            stream << " (" << name << ")";
        }
        stream << (iio_channel_is_output(channel) ? " [output]" : " [input]");

        auto scale = 0.0;
        auto offset = 0LL;
        if (readChannelAttrDouble(channel, "scale", scale)) {
            stream << " scale=" << scale;
        }
        if (readChannelAttrLongLong(channel, "offset", offset)) {
            stream << " offset=" << offset;
        }
        stream << '\n';
    }

    const auto attributes_count = iio_device_get_attrs_count(device_);
    if (attributes_count > 0) {
        stream << "Device attributes: " << attributes_count << '\n';
        for (unsigned int index = 0; index < attributes_count; ++index) {
            const char* attribute_name = iio_device_get_attr(device_, index);
            std::array<char, 256> buffer{};
            const auto ret = iio_device_attr_read(device_, attribute_name, buffer.data(), buffer.size());
            if (ret <= 0) {
                continue;
            }

            auto value = std::string(buffer.data(), static_cast<std::size_t>(ret));
            trimTrailingNewline(&value);
            stream << "  " << attribute_name << " = " << value << '\n';
        }
    }

    return stream.str();
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::availableSamplingFrequencies() const -> std::string {
    std::array<char, 256> buffer{};
    const auto ret =
            iio_device_attr_read(device_, "sampling_frequency_available", buffer.data(), buffer.size());

    if (ret <= 0) {
        return "unknown";
    }

    auto value = std::string(buffer.data(), static_cast<std::size_t>(ret));
    trimTrailingNewline(&value);
    return value;
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::setSamplingFrequency(int frequency_hz) const -> bool {
    const auto ret = iio_device_attr_write_longlong(device_, "sampling_frequency", frequency_hz);
    return (ret >= 0);
}

//-------------------------------------------------------------------------------------------------
auto HTS221Sensor::readSample(HTS221Data& data) const -> bool {
    auto humidity_raw = 0LL;
    auto temperature_raw = 0LL;

    if (iio_channel_attr_read_longlong(humidity_channel_, "raw", &humidity_raw) < 0) {
        return false;
    }

    if (iio_channel_attr_read_longlong(temperature_channel_, "raw", &temperature_raw) < 0) {
        return false;
    }

    auto humidity_scale = 1.0;
    auto temperature_scale = 1.0;
    auto humidity_offset = 0LL;
    auto temperature_offset = 0LL;

    std::ignore = readChannelAttrDouble(humidity_channel_, "scale", humidity_scale);
    std::ignore = readChannelAttrDouble(temperature_channel_, "scale", temperature_scale);
    std::ignore = readChannelAttrLongLong(humidity_channel_, "offset", humidity_offset);
    std::ignore = readChannelAttrLongLong(temperature_channel_, "offset", temperature_offset);

    data.humidity_percent = (static_cast<double>(humidity_raw + humidity_offset) * humidity_scale);
    data.temperature_celsius =
            (static_cast<double>(temperature_raw + temperature_offset) * temperature_scale);
    data.timestamp = std::chrono::system_clock::now();

    return true;
}

}  // namespace sense_hat::iio