#include "modern_scenegraph.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <memory>

// Global scene graph
std::unique_ptr<ModernSceneGraph> g_graph;

// Window and OpenGL context
SDL_Window* window = nullptr;
SDL_GLContext glContext;

// Window dimensions
int windowWidth = 800;
int windowHeight = 600;

// Camera state
struct ModernCamera {
    float yaw = -90.0f;
    float pitch = 0.0f;
    float radius = 5.0f;
    glm::vec3 target = glm::vec3(0.0f);
    float mouseSensitivity = 0.1f;
    float scrollSensitivity = 0.1f;
    
    glm::vec3 getPosition() const {
        float x = target.x + radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        float y = target.y + radius * sin(glm::radians(pitch));
        float z = target.z + radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::vec3(x, y, z);
    }
    
    glm::vec3 getDirection() const {
        return glm::normalize(target - getPosition());
    }
    
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(getPosition(), target, glm::vec3(0, 1, 0));
    }
    
    void processMouseMovement(float xoffset, float yoffset) {
        yaw += xoffset * mouseSensitivity;
        pitch += yoffset * mouseSensitivity;
        
        // Constrain pitch
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
    
    void processMouseScroll(float yoffset) {
        radius -= yoffset * scrollSensitivity;
        if (radius < 1.0f) radius = 1.0f;
        if (radius > 50.0f) radius = 50.0f;
    }
};

ModernCamera g_camera;

// Mouse state
bool mouseCaptured = true;

// Node IDs for dynamic updates
uint32_t camera_light_id = 0;
uint32_t moving_light_id = 0;

// Handle mouse motion
void handleMouseMotion(int xrel, int yrel) {
    if (!mouseCaptured) return;
    
    g_camera.processMouseMovement(xrel, -yrel);
}

// Handle mouse wheel
void handleMouseWheel(int yoffset) {
    if (!mouseCaptured) return;
    
    g_camera.processMouseScroll(yoffset);
}

// Handle key presses
void handleKeyPress(SDL_Keycode key) {
    if (key == SDLK_ESCAPE) {
        // Exit will be handled in main loop
    }
    else if (key == SDLK_TAB) {
        // Toggle mouse capture
        mouseCaptured = !mouseCaptured;
        if (mouseCaptured) {
            SDL_SetWindowRelativeMouseMode(window, true);
            std::cout << "Mouse captured - camera controls enabled\n";
        } else {
            SDL_SetWindowRelativeMouseMode(window, false);
            std::cout << "Mouse released - camera controls disabled\n";
        }
    }
}

// Handle window resize
void handleWindowResize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Update viewport
    glViewport(0, 0, width, height);
    
    // Update projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspectRatio = (float)width / (float)height;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    glLoadMatrixf(glm::value_ptr(projection));
    
    glMatrixMode(GL_MODELVIEW);
}

// Setup OpenGL state
void setupOpenGL() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    
    // Set clear color to dark blue
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    glLoadMatrixf(glm::value_ptr(projection));
    
    glMatrixMode(GL_MODELVIEW);
    
    // Enable normalization of normals
    glEnable(GL_NORMALIZE);
}

// Setup dynamic lights in the scene
void setupSceneLights(float time) {
    // Setup camera spotlight (follows camera)
    if (camera_light_id != 0) {
        glEnable(GL_LIGHT0);
        
        glm::vec3 lightPos = g_camera.getPosition();
        glm::vec3 lightDir = g_camera.getDirection();
        
        GLfloat position[] = {lightPos.x, lightPos.y, lightPos.z, 1.0f};
        GLfloat direction[] = {lightDir.x, lightDir.y, lightDir.z};
        GLfloat diffuse[] = {1.0f, 0.95f, 0.8f, 1.0f};
        GLfloat ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
        
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 2.0f);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.09f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.032f);
    }
    
    // Setup moving spotlight
    if (moving_light_id != 0) {
        glEnable(GL_LIGHT1);
        
        const float lightHeight = 8.0f;
        const float lightRadius = 4.0f;
        const float lightSpeed = 1.5f;
        
        float x = lightRadius * sin(time * lightSpeed);
        float z = lightRadius * cos(time * lightSpeed * 0.7f);
        glm::vec3 movingLightPos(x, lightHeight, z);
        glm::vec3 movingLightDir(0.0f, -1.0f, 0.0f);
        
        GLfloat position[] = {movingLightPos.x, movingLightPos.y, movingLightPos.z, 1.0f};
        GLfloat direction[] = {movingLightDir.x, movingLightDir.y, movingLightDir.z};
        GLfloat diffuse[] = {0.9f, 0.9f, 1.0f, 1.0f};
        GLfloat ambient[] = {0.05f, 0.05f, 0.1f, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, position);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 25.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 3.0f);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.07f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.017f);
    }
}

// Create the modern scene equivalent to the traditional OOP version
void createModernScene() {
    g_graph = std::make_unique<ModernSceneGraph>();
    uint32_t root = g_graph->getRootNode();
    
    // Create camera node (for reference, not used for actual camera)
    uint32_t camera = createCameraNode(*g_graph, "main_camera");
    g_graph->addChild(root, camera);
    
    // Create lights (store IDs for dynamic updates)
    camera_light_id = createLightNode(*g_graph, "camera_spotlight");
    LightComponent camera_light_comp;
    camera_light_comp.color = glm::vec3(1.0f, 0.95f, 0.8f);
    camera_light_comp.intensity = 1.2f;
    g_graph->addLightComponent(camera_light_id, camera_light_comp);
    g_graph->addChild(root, camera_light_id);
    
    moving_light_id = createLightNode(*g_graph, "moving_spotlight");
    LightComponent moving_light_comp;
    moving_light_comp.color = glm::vec3(0.9f, 0.9f, 1.0f);
    moving_light_comp.intensity = 2.0f;
    g_graph->addLightComponent(moving_light_id, moving_light_comp);
    g_graph->addChild(root, moving_light_id);
    
    // Create ground plane
    uint32_t ground = createGeometryNode(*g_graph, "ground", GeometryType::Plane);
    g_graph->addGeometryComponent(ground, createPlaneGeometry(20.0f, 20.0f, glm::vec3(0.3f, 0.3f, 0.3f)));
    g_graph->addChild(root, ground);
    
    // Create cone group with separator
    uint32_t cone_group = createSeparatorNode(*g_graph, "cone_group", true, false, false);
    g_graph->addChild(root, cone_group);
    
    // Create cones in a circle
    for (int i = 0; i < 5; ++i) {
        uint32_t cone_transform = createTransformNode(*g_graph, "cone" + std::to_string(i+1) + "_transform");
        float angle = (i * 72.0f) * M_PI / 180.0f;
        glm::vec3 position(2.5f * cos(angle), 0, 2.5f * sin(angle));
        g_graph->setTranslation(cone_transform, position);
        g_graph->addChild(cone_group, cone_transform);
        
        uint32_t cone = createGeometryNode(*g_graph, "cone" + std::to_string(i+1), GeometryType::Cone);
        glm::vec3 color(0.8f, 0.2f + i * 0.15f, 0.2f + i * 0.1f);
        g_graph->addGeometryComponent(cone, createConeGeometry(0.5f, 1.5f, 16, color));
        g_graph->addChild(cone_transform, cone);
    }
    
    // Create primitives group with color saving
    uint32_t primitives_group = createSeparatorNode(*g_graph, "primitives_group", true, false, true);
    g_graph->addChild(root, primitives_group);
    
    // Sphere with transform
    uint32_t sphere_transform = createTransformNode(*g_graph, "sphere_transform");
    g_graph->setTranslation(sphere_transform, glm::vec3(1.5f, 1.5f, 1.5f));
    g_graph->setRotation(sphere_transform, glm::vec3(0, 45, 0));
    g_graph->addChild(primitives_group, sphere_transform);
    
    uint32_t sphere = createGeometryNode(*g_graph, "sphere", GeometryType::Sphere);
    g_graph->addGeometryComponent(sphere, createSphereGeometry(0.7f, 32, 16, glm::vec3(0.8f, 0.2f, 0.8f)));
    g_graph->addChild(sphere_transform, sphere);
    
    // Cylinder with transform
    uint32_t cylinder_transform = createTransformNode(*g_graph, "cylinder_transform");
    g_graph->setTranslation(cylinder_transform, glm::vec3(-1.5f, 0, 1.5f));
    g_graph->setRotation(cylinder_transform, glm::vec3(15, 0, 0));
    g_graph->addChild(primitives_group, cylinder_transform);
    
    uint32_t cylinder = createGeometryNode(*g_graph, "cylinder", GeometryType::Cylinder);
    g_graph->addGeometryComponent(cylinder, createCylinderGeometry(0.5f, 1.5f, glm::vec3(0.2f, 0.8f, 0.8f)));
    g_graph->addChild(cylinder_transform, cylinder);
    
    // Cube with transform
    uint32_t cube_transform = createTransformNode(*g_graph, "cube_transform");
    g_graph->setTranslation(cube_transform, glm::vec3(1.5f, 0.5f, -1.5f));
    g_graph->setRotation(cube_transform, glm::vec3(0, 30, 0));
    g_graph->setScale(cube_transform, 1.2f);
    g_graph->addChild(primitives_group, cube_transform);
    
    uint32_t cube = createGeometryNode(*g_graph, "cube", GeometryType::Cube);
    g_graph->addGeometryComponent(cube, createCubeGeometry(1.0f, glm::vec3(0.9f, 0.5f, 0.1f)));
    g_graph->addChild(cube_transform, cube);
    
    // Complex hierarchical transforms
    uint32_t complex_group = createSeparatorNode(*g_graph, "complex_group", true, false, false);
    g_graph->addChild(root, complex_group);
    
    uint32_t parent_transform = createTransformNode(*g_graph, "parent_transform");
    g_graph->setTranslation(parent_transform, glm::vec3(-2.0f, 1.0f, -2.0f));
    g_graph->addChild(complex_group, parent_transform);
    
    uint32_t child_transform = createTransformNode(*g_graph, "child_transform");
    g_graph->setTranslation(child_transform, glm::vec3(0, 1.0f, 0));
    g_graph->setScale(child_transform, 0.9f);
    g_graph->addChild(parent_transform, child_transform);
    
    uint32_t sphere2 = createGeometryNode(*g_graph, "sphere2", GeometryType::Sphere);
    g_graph->addGeometryComponent(sphere2, createSphereGeometry(0.9f, 32, 16, glm::vec3(0.1f, 0.9f, 0.4f)));
    g_graph->addChild(child_transform, sphere2);
    
    // Nested group with complex transforms
    uint32_t nested_group = createSeparatorNode(*g_graph, "nested_group", true, false, false);
    g_graph->addChild(root, nested_group);
    
    uint32_t outer_transform = createTransformNode(*g_graph, "outer_transform");
    g_graph->setTranslation(outer_transform, glm::vec3(-3.0f, 0.6f, 0.5f));
    g_graph->setRotation(outer_transform, glm::vec3(0, 45, 0));
    g_graph->addChild(nested_group, outer_transform);
    
    uint32_t inner_transform = createTransformNode(*g_graph, "inner_transform");
    g_graph->setRotation(inner_transform, glm::vec3(45, 0, 45));
    g_graph->setScale(inner_transform, 0.8f);
    g_graph->addChild(outer_transform, inner_transform);
    
    uint32_t cube2 = createGeometryNode(*g_graph, "cube2", GeometryType::Cube);
    g_graph->addGeometryComponent(cube2, createCubeGeometry(1.0f, glm::vec3(0.7f, 0.1f, 0.7f)));
    g_graph->addChild(inner_transform, cube2);
}

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
        return -1;
    }
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // Create window
    window = SDL_CreateWindow("Modern Scene Graph Demo - SDL3 Version",
                             windowWidth, windowHeight,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }
    
    // Create OpenGL context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Enable VSync
    SDL_GL_SetSwapInterval(1);
    
    // Set initial mouse capture
    SDL_SetWindowRelativeMouseMode(window, true);

    setupOpenGL();

    // Create Scene
    createModernScene();
    
    // Print the scene hierarchy
    std::cout << "\n=== Modern Scene Graph Hierarchy ===\n";
    g_graph->printHierarchy();
    std::cout << "====================================\n\n";
    
    std::cout << "Mouse Controls:\n";
    std::cout << "- Move mouse to rotate camera around the scene (when captured)\n";
    std::cout << "- Scroll wheel to zoom in/out (when captured)\n";
    std::cout << "- Press TAB to toggle mouse capture on/off\n";
    std::cout << "- Press ESC to exit\n";
    std::cout << "- Resize the window as needed\n";
    std::cout << "- Watch the moving spotlight from above!\n";
    std::cout << "Mouse is initially captured. Press TAB to release it.\n";

    // Animation variables
    float time = 0.0f;
    
    // Main loop
    bool running = true;
    SDL_Event event;
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        time += deltaTime;
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                    
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    } else {
                        handleKeyPress(event.key.key);
                    }
                    break;
                    
                case SDL_EVENT_MOUSE_MOTION:
                    if (mouseCaptured) {
                        handleMouseMotion(event.motion.xrel, event.motion.yrel);
                    }
                    break;
                    
                case SDL_EVENT_MOUSE_WHEEL:
                    handleMouseWheel(event.wheel.y);
                    break;
                    
                case SDL_EVENT_WINDOW_RESIZED:
                    handleWindowResize(event.window.data1, event.window.data2);
                    break;
            }
        }
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        
        // Use the camera's view matrix
        glm::mat4 view = g_camera.getViewMatrix();
        glLoadMatrixf(glm::value_ptr(view));
        
        // Setup dynamic lights
        setupSceneLights(time);
        
        // Update and draw the scene
        g_graph->updateTransforms();
        g_graph->draw();

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    g_graph.reset();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
