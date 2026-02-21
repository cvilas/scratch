#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cstring>
#include <iostream>
#include <cinttypes>

class SenseHatLED {
private:
    int fbfd;
    uint16_t* framebuffer;
    const int width = 8;
    const int height = 8;
    
public:
    SenseHatLED() {
        fbfd = open("/dev/fb0", O_RDWR);
        if (fbfd == -1) {
            throw std::runtime_error("Cannot open framebuffer device");
        }
        
        // Map framebuffer to memory
        framebuffer = (uint16_t*)mmap(0, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
        if (framebuffer == MAP_FAILED) {
            close(fbfd);
            throw std::runtime_error("Failed to mmap framebuffer");
        }
    }
    
    ~SenseHatLED() {
        munmap(framebuffer, 128);
        close(fbfd);
    }
    
    // Convert RGB888 to RGB565
    uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    }
    
    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            framebuffer[y * width + x] = rgb565(r, g, b);
        }
    }
    
    void clear() {
        memset(framebuffer, 0, 128);
    }
    
    void showPattern() {
        // Example:  checkerboard pattern
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if ((x + y) % 2 == 0) {
                    setPixel(x, y, 255, 0, 0); // Red
                } else {
                    setPixel(x, y, 0, 0, 255); // Blue
                }
            }
        }
    }
};

int main() {
  SenseHatLED led;
  led.showPattern();
  sleep(1);
}
