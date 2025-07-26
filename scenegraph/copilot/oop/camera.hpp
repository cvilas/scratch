#pragma once
#include "node.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Camera node
class Camera : public Node {
public:
    // Camera parameters
    glm::vec3 position{0.0f, 2.0f, 5.0f};
    glm::vec3 target{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    
    // Mouse control parameters
    float yaw = -90.0f;   // Horizontal rotation
    float pitch = 0.0f;   // Vertical rotation
    float distance = 5.0f; // Distance from target
    
    // Sensitivity settings
    float mouseSensitivity = 0.1f;
    float scrollSensitivity = 0.5f;
    
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    
    Camera(const std::string& name_);
    
    void updateViewMatrix();
    void processMouseMovement(float xoffset, float yoffset);
    void processMouseScroll(float yoffset);
    
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getDirection() const;
    
    // No draw needed for camera in this simple demo
};
