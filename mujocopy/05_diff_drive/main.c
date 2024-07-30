

#include<stdbool.h> //for bool
//#include<unistd.h> //for usleep
//#include <math.h>

#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


char filename[] = "diff_drive.xml";

// MuJoCo data structures
mjModel* model = NULL;              // MuJoCo model
mjData* data = NULL;                // MuJoCo data
mjvCamera cam;                      // abstract camera
mjvOption opt;                      // visualization options
mjvScene scn;                       // abstract scene
mjrContext con;                     // custom GPU context

// mouse interaction
bool button_left = false;
bool button_middle = false;
bool button_right =  false;
double lastx = 0;
double lasty = 0;
double vf = 0;
double vr = 0;

// holders of one step history of time and position to calculate dertivatives
mjtNum position_history = 0;
mjtNum previous_time = 0;

// controller related variables
float_t ctrl_update_freq = 100;
mjtNum last_update = 0.0;
mjtNum ctrl;

// keyboard callback
void keyboard(GLFWwindow* window, int key, int scancode, int act, int mods) {
    if( act == GLFW_PRESS || act == GLFW_REPEAT) {
        switch(key){
            case GLFW_KEY_BACKSPACE:
                vf = 0;
                vr = 0;
                mj_resetData(model, data);
                mj_forward(model, data);
                break;
            case GLFW_KEY_UP:
                vf += 0.1;
                break;
            case GLFW_KEY_DOWN:
                vf -= 0.1;
                break;
            case GLFW_KEY_RIGHT:
                vr += 0.1;
                break;
            case GLFW_KEY_LEFT:
                vr -= 0.1;
                break;
        }
    }
}

// mouse button callback
void mouse_button(GLFWwindow* window, int button, int act, int mods) {
    // update button state
    button_left =   (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS);
    button_middle = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE)==GLFW_PRESS);
    button_right =  (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS);

    // update mouse position
    glfwGetCursorPos(window, &lastx, &lasty);
}


// mouse move callback
void mouse_move(GLFWwindow* window, double xpos, double ypos) {
    // no buttons down: nothing to do
    if( !button_left && !button_middle && !button_right )
        return;

    // compute mouse displacement, save
    double dx = xpos - lastx;
    double dy = ypos - lasty;
    lastx = xpos;
    lasty = ypos;

    // get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // get shift key state
    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS);

    // determine action based on mouse button
    mjtMouse action;
    if( button_right ) {
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    } else if( button_left ) {
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    } else {
        action = mjMOUSE_ZOOM;
    }

    // move camera
    mjv_moveCamera(model, action, dx/height, dy/height, &scn, &cam);
}


// scroll callback
void scroll(GLFWwindow* window, double xoffset, double yoffset) {
    // emulate vertical mouse motion = 5% of window height
    mjv_moveCamera(model, mjMOUSE_ZOOM, 0, -0.05*yoffset, &scn, &cam);
}


// main function
int main(int argc, const char** argv) {
    // load and compile model
    char error[1000] = "Could not load binary model";

    // check command-line arguments
    if( argc<2 ) {
        model = mj_loadXML(filename, 0, error, 1000);
    } else {
        if( strlen(argv[1])>4 && !strcmp(argv[1]+strlen(argv[1])-4, ".mjb") ) {
            model = mj_loadModel(argv[1], 0);
        } else {
            model = mj_loadXML(argv[1], 0, error, 1000);
        }
    }
    if( !model ) {
        mju_error_s("Load model error: %s", error);
    }

    // make data
    data = mj_makeData(model);

    // init GLFW
    if( !glfwInit() ) {
        mju_error("Could not initialize GLFW");
    }

    // create window, make OpenGL context current, request v-sync
    GLFWwindow* window = glfwCreateWindow(1244, 700, "Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // initialize visualization data structures
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);
    mjv_makeScene(model, &scn, 2000);                // space for 2000 objects
    mjr_makeContext(model, &con, mjFONTSCALE_150);   // model-specific context

    // install GLFW mouse and keyboard callbacks
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse_move);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetScrollCallback(window, scroll);

    double arr_view[] = {90, -45, 13, 0.000000, 0.000000, 0.000000};
    cam.azimuth = arr_view[0];
    cam.elevation = arr_view[1];
    cam.distance = arr_view[2];
    cam.lookat[0] = arr_view[3];
    cam.lookat[1] = arr_view[4];
    cam.lookat[2] = arr_view[5];

    // Get wheel separation
    int left_wheel_id = mj_name2id(model, mjOBJ_BODY, "left-tire");
    int right_wheel_id = mj_name2id(model, mjOBJ_BODY, "right-tire");
    double left_wheel_y = model->body_pos[left_wheel_id * 3 + 1];
    double right_wheel_y = model->body_pos[right_wheel_id * 3 + 1];
    double WHEEL_SEPARATION = fabs(left_wheel_y - right_wheel_y);

    // Get wheel radius
    int left_wheel_geom_id = model->body_geomadr[left_wheel_id];
    double WHEEL_RADIUS = model->geom_size[left_wheel_geom_id * 3  + 0];
    printf("Wheel separation: %.2f meters\n", WHEEL_SEPARATION);
    printf("Wheel radius: %d %.2f meters\n", left_wheel_geom_id, WHEEL_RADIUS);

    // use the first while condition if you want to simulate for a period.
    while( !glfwWindowShouldClose(window)) {
        // advance interactive simulation for 1/60 sec
        //  Assuming MuJoCo can simulate faster than real-time, which it usually can,
        //  this loop will finish on time for the next frame to be rendered at 60 fps.
        //  Otherwise add a cpu timer and exit this loop when it is time to render.
        mjtNum simstart = data->time;
        while( data->time - simstart < 1.0/60.0 ) {
            data->ctrl[0] = (2 * vf + vr * WHEEL_SEPARATION) / (2 * WHEEL_RADIUS);
            data->ctrl[1] = (2 * vf - vr * WHEEL_SEPARATION) / (2 * WHEEL_RADIUS);
            mj_step(model, data);
            printf("%f %f %f\n", data->site_xpos[0], data->site_xpos[1], data->site_xpos[2]);
        }

        // get framebuffer viewport
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

          // update scene and render
        mjv_updateScene(model, data, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);
        //printf("{%f, %f, %f, %f, %f, %f};\n",cam.azimuth,cam.elevation, cam.distance,cam.lookat[0],cam.lookat[1],cam.lookat[2]);

        // swap OpenGL buffers (blocking call due to v-sync)
        glfwSwapBuffers(window);

        // process pending GUI events, call GLFW callbacks
        glfwPollEvents();
    }

    // free visualization storage
    mjv_freeScene(&scn);
    mjr_freeContext(&con);

    // free MuJoCo model and data, deactivate
    mj_deleteData(data);
    mj_deleteModel(model);

    glfwTerminate();

    return 1;
}
