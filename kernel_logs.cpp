// reads linux kernel logs
// g++ -std=c++26 -O2 -Wall -Wextra -Wpedantic kernel_logs.cpp -o kernel_logs

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <exception>
#include <print>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

namespace {

    volatile std::sig_atomic_t stop_requested = 0;

    extern "C" void handleSignal(int) {
        stop_requested = 1;
    }

    [[noreturn]] void fail(const char* operation) {
        const int error = errno;
        throw std::system_error(error, std::generic_category(), operation);
    }

    class FileDescriptor {
    public:
        explicit FileDescriptor(const char* path)
        : fd_(::open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) {
            if (fd_ == -1) {
                fail("open /dev/kmsg");
            }
        }

        ~FileDescriptor() {
            ::close(fd_);
        }

        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor& operator=(const FileDescriptor&) = delete;

        [[nodiscard]] int get() const noexcept {
            return fd_;
        }

    private:
        int fd_;
    };

    void installSignalHandlers() {
        struct sigaction action {};
        action.sa_handler = handleSignal;
        ::sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        if (::sigaction(SIGINT, &action, nullptr) == -1 ||
            ::sigaction(SIGTERM, &action, nullptr) == -1) {
            fail("sigaction");
            }
    }

    void followKernelLogs(bool include_history) {
        const FileDescriptor device{"/dev/kmsg"};

        if (!include_history && ::lseek(device.get(), 0, SEEK_END) == -1) {
            fail("lseek /dev/kmsg");
        }

        // Generous buffer: /dev/kmsg reads return whole records, not chunks.
        std::vector<char> buffer(1024 * 1024);

        while (!stop_requested) {
            pollfd descriptor{device.get(), POLLIN, 0};

            // A finite timeout also bounds idle shutdown latency.
            const int ready = ::poll(&descriptor, 1, 250);

            if (ready == -1) {
                if (errno == EINTR) {
                    continue;
                }
                fail("poll /dev/kmsg");
            }

            if (stop_requested) {
                break;
            }

            if (ready == 0) {
                continue;
            }

            if ((descriptor.revents & (POLLNVAL | POLLHUP)) != 0) {
                throw std::runtime_error("/dev/kmsg became unavailable");
            }

            // Read even on POLLERR: an overrun is reported by read() as EPIPE.
            const auto count = ::read(device.get(), buffer.data(), buffer.size());

            if (count > 0) {
                const std::string_view record{
                    buffer.data(), static_cast<std::size_t>(count)
                };

                // Keep the kernel-provided escaping and metadata intact.
                std::print("{}", record);
                if (record.back() != '\n') {
                    std::print("\n");
                }

                if (std::fflush(stdout) == EOF) {
                    fail("flush stdout");
                }
                continue;
            }

            if (count == 0) {
                throw std::runtime_error("Unexpected EOF from /dev/kmsg");
            }

            const int error = errno;

            if (error == EINTR || error == EAGAIN) {
                continue;
            }

            if (error == EPIPE) {
                std::println(stderr,
                             "Warning: kernel log buffer overrun; records were lost.");
                continue;
            }

            if (error == EINVAL) {
                throw std::runtime_error(
                    "/dev/kmsg rejected the read; the record may exceed the buffer");
            }

            throw std::system_error(
                error, std::generic_category(), "read /dev/kmsg");
        }
    }

} // namespace

int main(int argc, char* argv[]) {
    try {
        const bool include_history =
        argc == 2 && std::string_view{argv[1]} == "--history";

        if (argc > 1 && !include_history) {
            std::println(stderr, "Usage: {} [--history]", argv[0]);
            return 1;
        }

        installSignalHandlers();
        followKernelLogs(include_history);
        return 0;
    } catch (const std::exception& error) {
        std::println(stderr, "Error: {}", error.what());
        return 1;
    }
}
