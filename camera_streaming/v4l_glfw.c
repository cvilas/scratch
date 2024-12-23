// Example program to capture image stream using V4L and display using GLFW
// sudo apt install libv4l-dev libglfw3-dev
// gcc -o v4l_glfw v4l_glfw.c -lglfw -lGL -lv4l2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <GLFW/glfw3.h>

#define CAMERA_DEVICE "/dev/video0"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BUFFER_COUNT 4

struct buffer {
    void *start;
    size_t length;
};

static struct buffer *buffers = NULL;
static int fd = -1;
static GLuint texture;

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

void init_camera() {
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;

    // Open camera device
    fd = open(CAMERA_DEVICE, O_RDWR);
    if (fd == -1) {
        perror("Cannot open camera device");
        exit(EXIT_FAILURE);
    }

    // Query camera capabilities
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("VIDIOC_QUERYCAP");
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "Device does not support video capture\n");
        exit(EXIT_FAILURE);
    }

    // Set format
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WINDOW_WIDTH;
    fmt.fmt.pix.height = WINDOW_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("VIDIOC_S_FMT");
        exit(EXIT_FAILURE);
    }

    // Request buffers
    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("VIDIOC_REQBUFS");
        exit(EXIT_FAILURE);
    }

    // Allocate buffers
    buffers = calloc(req.count, sizeof(*buffers));

    for (size_t i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("VIDIOC_QUERYBUF");
            exit(EXIT_FAILURE);
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               fd, buf.m.offset);

        if (buffers[i].start == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
    }
}

void start_capturing() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for (size_t i = 0; i < BUFFER_COUNT; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("VIDIOC_QBUF");
            exit(EXIT_FAILURE);
        }
    }

    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("VIDIOC_STREAMON");
        exit(EXIT_FAILURE);
    }
}

void init_gl() {
    // Generate texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Initialize texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT,
                 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

void convert_yuyv_to_rgb(const uint8_t *yuyv, uint8_t *rgb, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j += 2) {
            int y0 = yuyv[(i * width + j) * 2];
            int u = yuyv[(i * width + j) * 2 + 1];
            int y1 = yuyv[(i * width + j) * 2 + 2];
            int v = yuyv[(i * width + j) * 2 + 3];

            int c = y0 - 16;
            int d = u - 128;
            int e = v - 128;

            rgb[(i * width + j) * 3] =
                (uint8_t)((298 * c + 409 * e + 128) >> 8);              // R
            rgb[(i * width + j) * 3 + 1] =
                (uint8_t)((298 * c - 100 * d - 208 * e + 128) >> 8);    // G
            rgb[(i * width + j) * 3 + 2] =
                (uint8_t)((298 * c + 516 * d + 128) >> 8);              // B

            c = y1 - 16;
            rgb[(i * width + j + 1) * 3] =
                (uint8_t)((298 * c + 409 * e + 128) >> 8);              // R
            rgb[(i * width + j + 1) * 3 + 1] =
                (uint8_t)((298 * c - 100 * d - 208 * e + 128) >> 8);    // G
            rgb[(i * width + j + 1) * 3 + 2] =
                (uint8_t)((298 * c + 516 * d + 128) >> 8);              // B
        }
    }
}

int main() {
    GLFWwindow* window;

    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwSetErrorCallback(error_callback);

    // Create window
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Camera Feed", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize camera
    init_camera();
    start_capturing();

    // Initialize OpenGL texture
    init_gl();

    uint8_t *rgb_buffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 3);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // Dequeue buffer
        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            perror("VIDIOC_DQBUF");
            break;
        }

        // Convert YUYV to RGB
        convert_yuyv_to_rgb(buffers[buf.index].start, rgb_buffer,
                           WINDOW_WIDTH, WINDOW_HEIGHT);

        // Update texture
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT,
                     0, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer);

        // Queue buffer back
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("VIDIOC_QBUF");
            break;
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    for (size_t i = 0; i < BUFFER_COUNT; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }

    free(buffers);
    free(rgb_buffer);
    close(fd);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
