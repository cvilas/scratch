//g++ v4l_sdl2.cpp -o v4l_sdl2 -lSDL2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <SDL2/SDL.h>
#include <stdexcept>

#define BUFFER_COUNT 4
#define DEFAULT_DEVICE "/dev/video0"
#define WIDTH 640
#define HEIGHT 480

struct Buffer {
    void* start;
    size_t length;
};

class CameraViewer {
private:
    int fd;
    Buffer* buffers;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    bool running;

public:
    CameraViewer() : fd(-1), buffers(nullptr), window(nullptr),
    renderer(nullptr), texture(nullptr), running(true) {}

    ~CameraViewer() {
        cleanup();
    }

    void initialize() {
        // Open the device
        fd = open(DEFAULT_DEVICE, O_RDWR);
        if (fd < 0) {
            throw std::runtime_error("Cannot open device");
        }

        // Query device capabilities
        struct v4l2_capability cap;
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
            throw std::runtime_error("Cannot query capabilities");
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            throw std::runtime_error("Device does not support video capture");
        }

        // Set format
        struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = WIDTH;
        fmt.fmt.pix.height = HEIGHT;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
            throw std::runtime_error("Cannot set format");
        }

        // Request buffers
        struct v4l2_requestbuffers req = {0};
        req.count = BUFFER_COUNT;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
            throw std::runtime_error("Cannot request buffers");
        }

        // Map the buffers
        buffers = new Buffer[BUFFER_COUNT];
        for (unsigned int i = 0; i < BUFFER_COUNT; i++) {
            struct v4l2_buffer buf = {0};
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
                throw std::runtime_error("Cannot query buffer");
            }

            buffers[i].length = buf.length;
            buffers[i].start = mmap(nullptr, buf.length,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    fd, buf.m.offset);

            if (buffers[i].start == MAP_FAILED) {
                throw std::runtime_error("Cannot mmap buffer");
            }
        }

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL initialization failed");
        }

        window = SDL_CreateWindow("V4L2 Camera Viewer",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  WIDTH, HEIGHT,
                                  SDL_WINDOW_SHOWN);
        if (!window) {
            throw std::runtime_error("Cannot create window");
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            throw std::runtime_error("Cannot create renderer");
        }

        texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_YUY2,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    WIDTH, HEIGHT);
        if (!texture) {
            throw std::runtime_error("Cannot create texture");
        }
    }

    void start_capture() {
        // Queue the buffers
        for (unsigned int i = 0; i < BUFFER_COUNT; i++) {
            struct v4l2_buffer buf = {0};
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
                throw std::runtime_error("Cannot queue buffer");
            }
        }

        // Start streaming
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
            throw std::runtime_error("Cannot start streaming");
        }
    }

    void run() {
        start_capture();

        while (running) {
            handle_events();
            capture_and_display();
        }
    }

private:
    void handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }
    }

    void capture_and_display() {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // Dequeue a buffer
        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            throw std::runtime_error("Cannot dequeue buffer");
        }

        // Update texture with new frame
        SDL_UpdateTexture(texture, nullptr, buffers[buf.index].start, WIDTH * 2);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // Queue the buffer back
        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            throw std::runtime_error("Cannot requeue buffer");
        }
    }

    void cleanup() {
        if (fd >= 0) {
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl(fd, VIDIOC_STREAMOFF, &type);

            for (unsigned int i = 0; i < BUFFER_COUNT; i++) {
                if (buffers[i].start) {
                    munmap(buffers[i].start, buffers[i].length);
                }
            }
            close(fd);
        }

        delete[] buffers;

        if (texture) {
            SDL_DestroyTexture(texture);
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }
};

int main() {
    try {
        CameraViewer viewer;
        viewer.initialize();
        viewer.run();
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    return 0;
}
