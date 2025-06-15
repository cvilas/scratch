// g++ -std=c++23 js_event.cpp -o js_event


//#pragma once

#include <functional>
#include <filesystem>
#include <linux/input.h>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <system_error>
#include <cerrno>

class Joystick {
public:
    struct Event {
        uint32_t time_sec;
        uint32_t time_usec;
        uint16_t type;
        uint16_t code;
        int32_t  value;
    };

    // callback will be called for each event read from the device
    explicit Joystick(std::function<void(const Event&)>&& callback)
        : callback_(std::move(callback)), fd_(-1)
    {}

    // Opens the device at device_name, returns true on success
    bool open(const std::filesystem::path& device_name) {
        close();
        fd_ = ::open(device_name.c_str(), O_RDONLY | O_NONBLOCK);
        return fd_ >= 0;
    }

    // Returns the file descriptor for integration with select/poll/epoll
    int get_fd() const { return fd_; }

    // Reads all available events and triggers the callback for each one.
    // Should be called when fd is readable (e.g., after select/poll/epoll signals readiness)
    void process() {
        if (fd_ < 0) return;
        struct input_event ev;
        ssize_t rd;
        while ((rd = ::read(fd_, &ev, sizeof(ev))) == sizeof(ev)) {
            Event e{uint32_t(ev.time.tv_sec), uint32_t(ev.time.tv_usec), ev.type, ev.code, ev.value};
            callback_(e);
        }
        // rd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK): no more data
        // rd == 0: device closed
    }

    // Returns true if device is open
    bool is_open() const { return fd_ >= 0; }

    // Closes the device if open
    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    ~Joystick() { close(); }

    // Non-copyable, movable
    Joystick(const Joystick&) = delete;
    Joystick& operator=(const Joystick&) = delete;
    Joystick(Joystick&& other) noexcept
        : callback_(std::move(other.callback_)), fd_(other.fd_)
    {
        other.fd_ = -1;
    }
    Joystick& operator=(Joystick&& other) noexcept {
        if (this != &other) {
            close();
            callback_ = std::move(other.callback_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

private:
    std::function<void(const Event&)> callback_;
    int fd_;
};

//#include "joystick.hpp"
#include <atomic>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>

std::atomic_flag exit_flag;

int main() {
    Joystick js([](const Joystick::Event& ev) {
        std::cout << "type=" << ev.type << " code=" << ev.code << " value=" << ev.value << '\n';
    });
    if (!js.open("/dev/input/by-path/pci-0000:00:14.0-usbv2-0:5:1.0-event-joystick")) {
        std::cerr << "Failed to open joystick device\n";
        return -1;
    }
    int fd = js.get_fd();

    while (!exit_flag.test()) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        int ret = select(fd + 1, &fds, nullptr, nullptr, nullptr);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            js.process();
        }
    }
    return 0;
}
