#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const std::string& name_) : Node(name_) {
    updateViewMatrix();
}

void Camera::updateViewMatrix() {
    // Calculate camera position based on spherical coordinates around target
    float x = target.x + distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    float y = target.y + distance * sin(glm::radians(pitch));
    float z = target.z + distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    
    position = glm::vec3(x, y, z);
    viewMatrix = glm::lookAt(position, target, up);
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    // Constrain pitch to avoid camera flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    updateViewMatrix();
}

void Camera::processMouseScroll(float yoffset) {
    distance -= yoffset * scrollSensitivity;
    if (distance < 0.5f) distance = 0.5f;
    if (distance > 50.0f) distance = 50.0f;
    
    updateViewMatrix();
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMatrix;
}

glm::vec3 Camera::getPosition() const {
    return position;
}

glm::vec3 Camera::getDirection() const {
    return glm::normalize(target - position);
}
