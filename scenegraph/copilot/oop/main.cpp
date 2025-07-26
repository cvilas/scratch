#include "scenegraph.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>

// Global camera pointer for mouse callbacks
std::shared_ptr<Camera> g_camera;

// Window and OpenGL context
SDL_Window* window = nullptr;
SDL_GLContext glContext;

// Window dimensions
int windowWidth = 800;
int windowHeight = 600;

// Mouse state
bool mouseCaptured = true;  // Track if mouse is captured

// Handle mouse motion
void handleMouseMotion(int xrel, int yrel) {
    if (!mouseCaptured) return;  // Don't process mouse if not captured
    
    // In SDL relative mode, we get relative movement directly
    float xoffset = xrel;
    float yoffset = -yrel;  // Reversed since y-coordinates go from bottom to top

    if (g_camera) {
        g_camera->processMouseMovement(xoffset, yoffset);
    }
}

// Handle mouse wheel
void handleMouseWheel(int yoffset) {
    if (!mouseCaptured) return;  // Don't process scroll if mouse not captured
    
    if (g_camera) {
        g_camera->processMouseScroll(yoffset);
    }
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

// Simple OpenGL setup for demonstration
void setupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    
    // Set viewport to fill the window
    glViewport(0, 0, windowWidth, windowHeight);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Use GLM instead of GLU for perspective projection with proper aspect ratio
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    glLoadMatrixf(glm::value_ptr(projection));
    
    glMatrixMode(GL_MODELVIEW);
    
    // Enable lighting and materials
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1); // Enable second light for moving spotlight
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Set up global ambient lighting (reduced to preserve colors)
    GLfloat globalAmbient[] = {0.25f, 0.25f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    // Enable two-sided lighting for better illumination
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    // Enable smooth shading and basic lighting setup
    glShadeModel(GL_SMOOTH);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);  // Very dark blue background for contrast
    
    // Enable depth testing but disable back-face culling for now (causing issues with plane)
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    
    // Enable normalization of normals (important for proper lighting)
    glEnable(GL_NORMALIZE);
}

void print_scene_hierarchy(std::shared_ptr<Node> node, int depth = 0) {
    std::string indent(depth * 2, ' ');
    
    // Get node type and additional info
    std::string nodeInfo = node->name;
    
    // Check if it's a Transform node and show transform info
    auto transform = std::dynamic_pointer_cast<Transform>(node);
    if (transform) {
        nodeInfo += " [Transform]";
    }
    
    // Check if it's a Separator node and show separator info
    auto separator = std::dynamic_pointer_cast<Separator>(node);
    if (separator) {
        nodeInfo += " [Separator";
        if (separator->saveTransform) nodeInfo += " +Transform";
        if (separator->saveMaterial) nodeInfo += " +Material";
        if (separator->saveColor) nodeInfo += " +Color";
        nodeInfo += "]";
    }
    
    // Check if it's a geometry node
    if (std::dynamic_pointer_cast<Cone>(node) ||
        std::dynamic_pointer_cast<Sphere>(node) ||
        std::dynamic_pointer_cast<Cylinder>(node) ||
        std::dynamic_pointer_cast<Cube>(node) ||
        std::dynamic_pointer_cast<Plane>(node)) {
        nodeInfo += " [Geometry]";
    }
    
    std::cout << indent << "- " << nodeInfo << std::endl;
    
    // Recursively print children
    for (const auto& child : node->children) {
        print_scene_hierarchy(child, depth + 1);
    }
}

// Example usage: Cone, Camera, Light
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
    window = SDL_CreateWindow("SceneGraph Demo - SDL3 Version",
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
    auto scene = std::make_shared<Scene>();

    // Camera
    auto camera = std::make_shared<Camera>("main_camera");
    g_camera = camera;  // Set global camera for mouse callbacks
    scene->addChild(camera);

    // Light (will be positioned at camera)
    auto light = std::make_shared<Light>("camera_spotlight");
    light->color = glm::vec3(1.0f, 0.95f, 0.8f); // Warm white light
    light->intensity = 1.2f;
    scene->addChild(light);
    
    // Moving spotlight from above
    auto movingLight = std::make_shared<Light>("moving_spotlight");
    movingLight->color = glm::vec3(0.8f, 0.9f, 1.0f); // Cool blue-white light
    movingLight->intensity = 1.0f;
    scene->addChild(movingLight);
    
    // Add ground plane
    auto ground = std::make_shared<Plane>("ground");
    ground->localTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.0f, 0)); // Raised slightly
    ground->color = glm::vec3(0.8f, 0.8f, 0.8f); // Brighter
    ground->size = 15.0f; // Larger for better visibility
    scene->addChild(ground);

    // Add multiple cones using the new Transform nodes to decouple geometry from transforms
    // Group 1: Using Separator and Transform for better organization
    auto coneGroup = std::make_shared<Separator>("cone_group");
    scene->addChild(coneGroup);
    
    // Cone 1 - at origin
    auto cone1Transform = std::make_shared<Transform>("cone1_transform");
    cone1Transform->setTranslation(glm::vec3(0, 0, 0));
    coneGroup->addChild(cone1Transform);
    
    auto cone1 = std::make_shared<Cone>("cone1");
    cone1->color = glm::vec3(1.0f, 0.3f, 0.1f);  // Red-orange
    cone1Transform->addChild(cone1);
    
    // Cone 2 - to the right
    auto cone2Transform = std::make_shared<Transform>("cone2_transform");
    cone2Transform->setTranslation(glm::vec3(2.5f, 0, 0));
    cone2Transform->setRotation(glm::vec3(0, 45, 0)); // Rotate 45 degrees around Y
    coneGroup->addChild(cone2Transform);
    
    auto cone2 = std::make_shared<Cone>("cone2");
    cone2->color = glm::vec3(0.1f, 0.8f, 0.2f);  // Green
    cone2Transform->addChild(cone2);
    
    // Cone 3 - to the left with scaling
    auto cone3Transform = std::make_shared<Transform>("cone3_transform");
    cone3Transform->setTranslation(glm::vec3(-2.5f, 0, 0));
    cone3Transform->setScale(1.5f); // Make it bigger
    coneGroup->addChild(cone3Transform);
    
    auto cone3 = std::make_shared<Cone>("cone3");
    cone3->color = glm::vec3(0.2f, 0.1f, 0.9f);  // Blue-purple
    cone3Transform->addChild(cone3);
    
    // Cone 4 - forward with complex transformation
    auto cone4Transform = std::make_shared<Transform>("cone4_transform");
    cone4Transform->setTranslation(glm::vec3(0, 0, 2.5f));
    cone4Transform->setRotation(glm::vec3(0, 0, 30)); // Tilt it
    cone4Transform->setScale(glm::vec3(0.8f, 1.2f, 0.8f)); // Non-uniform scale
    coneGroup->addChild(cone4Transform);
    
    auto cone4 = std::make_shared<Cone>("cone4");
    cone4->color = glm::vec3(0.9f, 0.1f, 0.5f);  // Pink
    cone4Transform->addChild(cone4);
    
    // Cone 5 - backward, simple transform
    auto cone5Transform = std::make_shared<Transform>("cone5_transform");
    cone5Transform->setTranslation(glm::vec3(0, 0, -2.5f));
    coneGroup->addChild(cone5Transform);
    
    auto cone5 = std::make_shared<Cone>("cone5");
    cone5->color = glm::vec3(0.8f, 0.8f, 0.1f);  // Yellow
    cone5Transform->addChild(cone5);
    
    // Add new primitive shapes using Transform and Separator nodes
    // Group 2: Mixed primitive shapes with state isolation
    auto primitivesGroup = std::make_shared<Separator>("primitives_group");
    primitivesGroup->enableColorSaving(); // Save color state for this group
    scene->addChild(primitivesGroup);
    
    // Sphere with transform
    auto sphereTransform = std::make_shared<Transform>("sphere_transform");
    sphereTransform->setTranslation(glm::vec3(1.5f, 1.5f, 1.5f));
    sphereTransform->setRotation(glm::vec3(0, 45, 0));
    primitivesGroup->addChild(sphereTransform);
    
    auto sphere = std::make_shared<Sphere>("sphere");
    sphere->color = glm::vec3(0.8f, 0.2f, 0.8f);  // Magenta
    sphere->radius = 0.7f;
    sphereTransform->addChild(sphere);
    
    // Cylinder with transform
    auto cylinderTransform = std::make_shared<Transform>("cylinder_transform");
    cylinderTransform->setTranslation(glm::vec3(-1.5f, 0, 1.5f));
    cylinderTransform->setRotation(glm::vec3(15, 0, 0)); // Slight tilt
    primitivesGroup->addChild(cylinderTransform);
    
    auto cylinder = std::make_shared<Cylinder>("cylinder");
    cylinder->color = glm::vec3(0.2f, 0.8f, 0.8f);  // Cyan
    cylinder->radius = 0.5f;
    cylinder->height = 1.5f;
    cylinderTransform->addChild(cylinder);
    
    // Cube with transform
    auto cubeTransform = std::make_shared<Transform>("cube_transform");
    cubeTransform->setTranslation(glm::vec3(1.5f, 0.5f, -1.5f));
    cubeTransform->setRotation(glm::vec3(0, 30, 0));
    cubeTransform->setScale(1.2f);
    primitivesGroup->addChild(cubeTransform);
    
    auto cube = std::make_shared<Cube>("cube");
    cube->color = glm::vec3(0.9f, 0.5f, 0.1f);  // Orange
    cubeTransform->addChild(cube);
    
    // Group 3: Complex hierarchical transforms
    auto complexGroup = std::make_shared<Separator>("complex_group");
    scene->addChild(complexGroup);
    
    // Parent transform for the whole group
    auto parentTransform = std::make_shared<Transform>("parent_transform");
    parentTransform->setTranslation(glm::vec3(-2.0f, 1.0f, -2.0f));
    complexGroup->addChild(parentTransform);
    
    // Child transform relative to parent
    auto childTransform = std::make_shared<Transform>("child_transform");
    childTransform->setTranslation(glm::vec3(0, 1.0f, 0)); // Offset upward from parent
    childTransform->setScale(0.9f);
    parentTransform->addChild(childTransform);
    
    auto sphere2 = std::make_shared<Sphere>("sphere2");
    sphere2->color = glm::vec3(0.1f, 0.9f, 0.4f);  // Bright green
    sphere2->radius = 0.9f;
    sphere2->slices = 32;  // Higher detail
    sphere2->stacks = 16;
    childTransform->addChild(sphere2);
    
    // Another cube with complex nested transforms
    auto nestedGroup = std::make_shared<Separator>("nested_group");
    scene->addChild(nestedGroup);
    
    auto outerTransform = std::make_shared<Transform>("outer_transform");
    outerTransform->setTranslation(glm::vec3(-3.0f, 0.6f, 0.5f));
    outerTransform->setRotation(glm::vec3(0, 45, 0));
    nestedGroup->addChild(outerTransform);
    
    auto innerTransform = std::make_shared<Transform>("inner_transform");
    innerTransform->setRotation(glm::vec3(45, 0, 45)); // Additional rotation
    innerTransform->setScale(0.8f);
    outerTransform->addChild(innerTransform);
    
    auto cube2 = std::make_shared<Cube>("cube2");
    cube2->color = glm::vec3(0.7f, 0.1f, 0.7f);  // Purple
    innerTransform->addChild(cube2);
    
    // Print the scene hierarchy to show the new Transform/Separator pattern
    std::cout << "\n=== Scene Graph Hierarchy ===\n";
    print_scene_hierarchy(scene);
    std::cout << "============================\n\n";
    
    std::cout << "Mouse Controls:\n";
    std::cout << "- Move mouse to rotate camera around the scene (when captured)\n";
    std::cout << "- Scroll wheel to zoom in/out (when captured)\n";
    std::cout << "- Press TAB to toggle mouse capture on/off\n";
    std::cout << "- Press ESC to exit\n";
    std::cout << "- Resize the window as needed\n";
    std::cout << "- Watch the moving spotlight from above!\n";
    std::cout << "Mouse is initially captured. Press TAB to release it.\n";

    // Animation variables for moving light
    float time = 0.0f;
    const float lightHeight = 8.0f;
    const float lightRadius = 4.0f;
    const float lightSpeed = 1.5f;

    // Main loop
    bool running = true;
    SDL_Event event;
    
    while (running) {
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
        
        // Update time for animation
        time += 0.016f; // Approximately 60 FPS
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        
        // Use the camera's view matrix
        glm::mat4 view = camera->getViewMatrix();
        glLoadMatrixf(glm::value_ptr(view));
        
        // Update camera spotlight to follow camera
        light->setupSpotlight(camera->getPosition(), camera->getDirection());
        
        // Calculate moving spotlight position (sinusoidal movement)
        float x = lightRadius * sin(time * lightSpeed);
        float z = lightRadius * cos(time * lightSpeed * 0.7f); // Different frequency for interesting pattern
        glm::vec3 movingLightPos(x, lightHeight, z);
        glm::vec3 movingLightDir(0.0f, -1.0f, 0.0f); // Looking straight down
        
        // Update moving spotlight
        movingLight->setupMovingSpotlight(movingLightPos, movingLightDir, GL_LIGHT1);

        scene->draw(glm::mat4(1.0f));

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}