/*
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libglfw3-dev libglew-dev

*/

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <mutex>
#include <vector>

// Globals
std::vector<uint8_t> frame_data;
int frame_width = 640;
int frame_height = 480;
std::mutex frame_mutex;

// Callback for new sample from appsink
GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data) {
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) return GST_FLOW_OK;

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *s = gst_caps_get_structure(caps, 0);

    gst_structure_get_int(s, "width", &frame_width);
    gst_structure_get_int(s, "height", &frame_height);

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame_data.assign(map.data, map.data + map.size);
        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

// OpenGL texture update
void update_texture(GLuint texture_id) {
    std::lock_guard<std::mutex> lock(frame_mutex);
    if (!frame_data.empty()) {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height,
                     0, GL_RGB, GL_UNSIGNED_BYTE, frame_data.data());
    }
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // GStreamer pipeline
    GstElement *pipeline = gst_parse_launch(
        "v4l2src ! videoconvert ! video/x-raw,format=RGB,width=640,height=480 ! appsink name=sink", nullptr);

    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    gst_app_sink_set_emit_signals((GstAppSink *)appsink, true);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample), nullptr);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // GLFW init
    if (!glfwInit()) return -1;
    GLFWwindow *window = glfwCreateWindow(640, 480, "Camera Viewer", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    GLuint texture;
    glGenTextures(1, &texture);

    // Texture setup
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        update_texture(texture);

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(-1, -1);
        glTexCoord2f(1, 1); glVertex2f(1, -1);
        glTexCoord2f(1, 0); glVertex2f(1, 1);
        glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    glfwTerminate();
    return 0;
}
