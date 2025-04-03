#include <iostream>
#include <csignal>
#include <cxxabi.h>
#include <execinfo.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <exception>
#include <cstring>

// Maximum number of stack frames to print
const int MAX_STACK_FRAMES = 64;

// Function to demangle C++ symbol names
std::string demangle(const char* symbol) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
    if (demangled != nullptr) {
        std::string result(demangled);
        free(demangled);
        return result;
    }
    return std::string(symbol);
}

// Function to print a stack trace
void print_stack_trace() {
    void* addr_list[MAX_STACK_FRAMES];

    // Get the stack addresses
    int addr_count = backtrace(addr_list, MAX_STACK_FRAMES);
    if (addr_count == 0) {
        std::cerr << "  <empty stack trace, possibly corrupt>" << std::endl;
        return;
    }

    // Get the stack traces as strings
    char** symbols = backtrace_symbols(addr_list, addr_count);
    if (symbols == nullptr) {
        std::cerr << "  <failed to get backtrace symbols>" << std::endl;
        return;
    }

    // Print the stack trace
    std::cerr << "Stack trace:" << std::endl;
    for (int i = 1; i < addr_count; i++) {  // Start from 1 to skip this function
        std::string symbol(symbols[i]);

        // Try to parse the line to extract the symbol
        size_t begin = symbol.find('(');
        size_t end = symbol.find('+', begin);

        if (begin != std::string::npos && end != std::string::npos) {
            std::string symname = symbol.substr(begin + 1, end - begin - 1);
            if (!symname.empty()) {
                symbol = symbol.substr(0, begin) + "(" + demangle(symname.c_str()) +
                         symbol.substr(end);
            }
        }

        std::cerr << "  #" << i << ": " << symbol << std::endl;
    }

    // Free the memory allocated by backtrace_symbols
    free(symbols);
}

// Signal handler function
void signal_handler(int signum) {
    std::cerr << "\n====== CRASH DETECTED ======" << std::endl;
    std::cerr << "Signal: " << signum << " (" << strsignal(signum) << ")" << std::endl;
    print_stack_trace();
    std::cerr << "============================" << std::endl;

    // Reset signal handler to default and re-raise the signal
    signal(signum, SIG_DFL);
    raise(signum);
}

// Terminate handler for uncaught exceptions
void terminate_handler() {
    std::cerr << "\n====== UNHANDLED EXCEPTION ======" << std::endl;

    // Try to get the current exception
    std::exception_ptr eptr = std::current_exception();
    if (eptr) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception type" << std::endl;
        }
    }

    print_stack_trace();
    std::cerr << "=================================" << std::endl;
    abort();
}

// Function to set up the termination handlers
void setup_termination_handlers() {
    // Set up the terminate handler for uncaught exceptions
    std::set_terminate(terminate_handler);

    // Set up signal handlers for common crash signals
    signal(SIGSEGV, signal_handler); // Segmentation fault
    signal(SIGILL, signal_handler);  // Illegal instruction
    signal(SIGFPE, signal_handler);  // Floating point exception
    signal(SIGABRT, signal_handler); // Abort
    signal(SIGBUS, signal_handler);  // Bus error

    // You can add more signals as needed
}

// Example main function to demonstrate usage
int main() {
    // Set up the termination handlers
    setup_termination_handlers();

    std::cout << "Termination handlers installed." << std::endl;

    // Uncomment one of these to test different crash scenarios:

    // 1. Segmentation fault
    // int* p = nullptr;
    // *p = 42;  // This will cause a segmentation fault

    // 2. Uncaught exception
    // throw std::runtime_error("This is an uncaught exception");

    // Normal program execution
    std::cout << "Program running normally." << std::endl;
    return 0;
}
