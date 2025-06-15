// g++ -std=c++23 js_event_with_init.cpp -o js_event_with_init

//#pragma once

#include <functional>
#include <filesystem>
#include <linux/input.h>
#include <linux/joystick.h>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cstring>
#include <system_error>
#include <vector>

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

    Joystick() : fd_(-1), epoll_fd_(-1), initial_state_reported_(false) {}

    ~Joystick() {
        close();
    }

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
        ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
        ev.data.fd = fd_;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd_, &ev) == -1) {
            ::close(fd_);
            ::close(epoll_fd_);
            fd_ = -1;
            epoll_fd_ = -1;
            return false;
        }
        initial_state_reported_ = false;
        return true;
    }

    state wait(std::function<void(const Event&)> callback, int timeout_ms = -1) {
        if (fd_ < 0 || epoll_fd_ < 0) return state::DISCONNECTED;

        // On first call, synthesise initial state events
        if (!initial_state_reported_) {
            report_initial_state(callback);
            initial_state_reported_ = true;
        }

        struct epoll_event ev;
        int nfds = epoll_wait(epoll_fd_, &ev, 1, timeout_ms);
        if (nfds == 0) {
            return state::TIMEOUT;
        }
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

    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        if (epoll_fd_ >= 0) {
            ::close(epoll_fd_);
            epoll_fd_ = -1;
        }
        initial_state_reported_ = false;
    }

    Joystick(const Joystick&) = delete;
    Joystick& operator=(const Joystick&) = delete;

private:
    int fd_;
    int epoll_fd_;
    bool initial_state_reported_;

    void report_initial_state(std::function<void(const Event&)> &callback) {
        // Report button state
        unsigned char keys[KEY_MAX/8 + 1];
        std::memset(keys, 0, sizeof(keys));
        if (ioctl(fd_, EVIOCGKEY(sizeof(keys)), keys) >= 0) {
            for (int code = 0; code < KEY_MAX; ++code) {
                if (ioctl(fd_, EVIOCGBIT(EV_KEY, KEY_MAX), nullptr) >= 0) {
                    int byte = code / 8;
                    int bit = code % 8;
                    bool pressed = (keys[byte] & (1 << bit));
                    // Only report if code is actually supported as a button
                    // Optionally, filter for joystick/gamepad button ranges
                    if (pressed || true) {
                        Event e{0, 0, EV_KEY, static_cast<uint16_t>(code), pressed ? 1 : 0};
                        callback(e);
                    }
                }
            }
        }

        // Report axis state
        unsigned long absbits[(ABS_MAX / (8 * sizeof(unsigned long))) + 1]{};
        if (ioctl(fd_, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits) >= 0) {
            for (int code = 0; code < ABS_MAX; ++code) {
                if (absbits[code / (8 * sizeof(unsigned long))] & (1UL << (code % (8 * sizeof(unsigned long))))) {
                    struct input_absinfo absinfo;
                    if (ioctl(fd_, EVIOCGABS(code), &absinfo) >= 0) {
                        Event e{0, 0, EV_ABS, static_cast<uint16_t>(code), absinfo.value};
                        callback(e);
                    }
                }
            }
        }
    }
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

