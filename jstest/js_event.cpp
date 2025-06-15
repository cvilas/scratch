// g++ -std=c++23 js_event.cpp -o js_event

//#pragma once

#include <functional>
#include <filesystem>
#include <linux/input.h>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <system_error>

class Joystick {
public:
    struct Event {
        uint32_t time_sec;
        uint32_t time_usec;
        uint16_t type;
        uint16_t code;
        int32_t  value;
    };

    enum class state {
        SUCCESS,
        DISCONNECTED,
        TIMEOUT
    };

    Joystick() : fd_(-1), epoll_fd_(-1) {}

    ~Joystick() {
        close();
    }

    // Opens the device at device_path, returns true on success
    bool open(std::filesystem::path& device_path) {
        close();
        fd_ = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd_ < 0)
            return false;

        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ < 0) {
            ::close(fd_);
            fd_ = -1;
            return false;
        }

        struct epoll_event ev;
        // Monitor for input, hang-up, and error events
        ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
        ev.data.fd = fd_;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd_, &ev) == -1) {
            ::close(fd_);
            ::close(epoll_fd_);
            fd_ = -1;
            epoll_fd_ = -1;
            return false;
        }
        return true;
    }

    // Waits for input events and invokes the callback for each event.
    // Returns state::SUCCESS if events were processed,
    // or state::DISCONNECTED if the device was disconnected or on error.
state wait(std::function<void(const Event&)> callback, int timeout_ms = -1) {
    if (fd_ < 0 || epoll_fd_ < 0) return state::DISCONNECTED;

    struct epoll_event ev;
    int nfds = epoll_wait(epoll_fd_, &ev, 1, timeout_ms);
    //if (nfds == 0) {
    //    return state::TIMEOUT;
    //}
    if (nfds < 0) {
        return state::DISCONNECTED;
    }
    if (ev.data.fd == fd_) {
        if (ev.events & (EPOLLHUP | EPOLLERR)) {
            close();
            return state::DISCONNECTED;
        }
        struct input_event iev;
        ssize_t rd;
        while ((rd = ::read(fd_, &iev, sizeof(iev))) == sizeof(iev)) {
            Event e{uint32_t(iev.time.tv_sec), uint32_t(iev.time.tv_usec), iev.type, iev.code, iev.value};
            callback(e);
        }
        if (rd == 0) {
            close();
            return state::DISCONNECTED;
        }
    }
    return state::SUCCESS;
}
    // Closes the device if open
    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        if (epoll_fd_ >= 0) {
            ::close(epoll_fd_);
            epoll_fd_ = -1;
        }
    }

    // Not copyable or movable
    Joystick(const Joystick&) = delete;
    Joystick& operator=(const Joystick&) = delete;

private:
    int fd_;
    int epoll_fd_;
};

//#include "joystick.hpp"
#include <iostream>
#include <atomic>

std::atomic_flag exit_flag;

int main() {
    Joystick js;
    std::filesystem::path dev = "/dev/input/by-path/pci-0000:00:14.0-usbv2-0:5:1.0-event-joystick";
    if (!js.open(dev)) {
        std::cerr << "Failed to open joystick device\n";
        return -1;
    }
    while (!exit_flag.test()) {
        auto result = js.wait([](const Joystick::Event& ev) {
            std::cout << "type=" << ev.type << " code=" << ev.code << " value=" << ev.value << '\n';
        }, -1);
        if (result == Joystick::state::DISCONNECTED) {
            std::cerr << "Joystick disconnected or error occurred\n";
            break;
        }
    }
    return 0;
}
