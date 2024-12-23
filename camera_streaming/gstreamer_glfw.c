// Image capture and streaming using gstreamer and glfw
// sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-base-apps libglew-dev
// gcc -o gstreamer_glfw gstreamer_glfw.c `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 glew` -lglfw -lGL

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold our application data
typedef struct {
    GstElement *pipeline;
    GstElement *appsink;
    GLFWwindow *window;
    GLuint texture;
    int width;
    int height;
} CameraViewer;

// GLFW error callback
static void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Initialize GStreamer pipeline
static int init_gstreamer(CameraViewer *viewer) {
    // Create pipeline elements
    viewer->pipeline = gst_pipeline_new("camera-pipeline");
    GstElement *source = gst_element_factory_make("v4l2src", "source");
    GstElement *convert = gst_element_factory_make("videoconvert", "convert");
    viewer->appsink = gst_element_factory_make("appsink", "sink");

    if (!viewer->pipeline || !source || !convert || !viewer->appsink) {
        fprintf(stderr, "One or more elements could not be created\n");
        return 0;
    }

    // Configure appsink
    gst_app_sink_set_emit_signals(GST_APP_SINK(viewer->appsink), TRUE);
    gst_app_sink_set_drop(GST_APP_SINK(viewer->appsink), TRUE);
    gst_app_sink_set_max_buffers(GST_APP_SINK(viewer->appsink), 1);

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(viewer->pipeline), source, convert, viewer->appsink, NULL);
    if (!gst_element_link_many(source, convert, viewer->appsink, NULL)) {
        fprintf(stderr, "Elements could not be linked\n");
        return 0;
    }

    // Set pipeline state to playing
    GstStateChangeReturn ret = gst_element_set_state(viewer->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        fprintf(stderr, "Unable to set the pipeline to the playing state\n");
        return 0;
    }

    return 1;
}

// Initialize GLFW and OpenGL
static int init_glfw(CameraViewer *viewer) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 0;
    }

    glfwSetErrorCallback(error_callback);

    viewer->window = glfwCreateWindow(viewer->width, viewer->height,
                                    "Camera Viewer", NULL, NULL);
    if (!viewer->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(viewer->window);
    glfwSwapInterval(1);

    // Initialize OpenGL texture
    glGenTextures(1, &viewer->texture);
    glBindTexture(GL_TEXTURE_2D, viewer->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return 1;
}

// Initialize the camera viewer
static CameraViewer* camera_viewer_create() {
    CameraViewer *viewer = (CameraViewer*)calloc(1, sizeof(CameraViewer));
    if (!viewer) {
        return NULL;
    }

    viewer->width = 640;
    viewer->height = 480;

    // Initialize GStreamer
    gst_init(NULL, NULL);

    if (!init_gstreamer(viewer)) {
        free(viewer);
        return NULL;
    }

    if (!init_glfw(viewer)) {
        gst_element_set_state(viewer->pipeline, GST_STATE_NULL);
        gst_object_unref(viewer->pipeline);
        free(viewer);
        return NULL;
    }

    return viewer;
}

// Clean up resources
static void camera_viewer_destroy(CameraViewer *viewer) {
    if (!viewer) return;

    if (viewer->pipeline) {
        gst_element_set_state(viewer->pipeline, GST_STATE_NULL);
        gst_object_unref(viewer->pipeline);
    }

    if (viewer->texture) {
        glDeleteTextures(1, &viewer->texture);
    }

    if (viewer->window) {
        glfwDestroyWindow(viewer->window);
    }

    glfwTerminate();
    free(viewer);
}

// Main rendering loop
static void camera_viewer_run(CameraViewer *viewer) {
    while (!glfwWindowShouldClose(viewer->window)) {
        // Pull sample from appsink
        GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(viewer->appsink));
        if (sample) {
            GstBuffer *buffer = gst_sample_get_buffer(sample);
            GstMapInfo map;
            if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
                // Update texture with new frame
                glBindTexture(GL_TEXTURE_2D, viewer->texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewer->width, viewer->height,
                            0, GL_RGB, GL_UNSIGNED_BYTE, map.data);
                gst_buffer_unmap(buffer, &map);
            }
            gst_sample_unref(sample);
        }

        // Render
        int window_width, window_height;
        glfwGetFramebufferSize(viewer->window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);

        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, viewer->texture);

        // Draw textured quad
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
        glEnd();

        glfwSwapBuffers(viewer->window);
        glfwPollEvents();
    }
}

int main(int argc, char *argv[]) {
    CameraViewer *viewer = camera_viewer_create();
    if (!viewer) {
        fprintf(stderr, "Failed to create camera viewer\n");
        return 1;
    }

    camera_viewer_run(viewer);
    camera_viewer_destroy(viewer);

    return 0;
}
