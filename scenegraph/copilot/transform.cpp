#include "transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform(const std::string& name_) : Node(name_) {
    updateTransform();
}

void Transform::setTranslation(const glm::vec3& t) {
    translation = t;
    updateTransform();
}

void Transform::setRotation(const glm::vec3& r) {
    rotation = r;
    updateTransform();
}

void Transform::setScale(const glm::vec3& s) {
    scale = s;
    updateTransform();
}

void Transform::setScale(float uniform_scale) {
    scale = glm::vec3(uniform_scale);
    updateTransform();
}

void Transform::translate(const glm::vec3& delta) {
    translation += delta;
    updateTransform();
}

void Transform::rotate(const glm::vec3& delta) {
    rotation += delta;
    updateTransform();
}

void Transform::scaleBy(const glm::vec3& factor) {
    scale *= factor;
    updateTransform();
}

void Transform::scaleBy(float factor) {
    scale *= factor;
    updateTransform();
}

void Transform::updateTransform() {
    // Build transformation matrix: T * R * S
    glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
    R = glm::rotate(R, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    R = glm::rotate(R, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    
    localTransform = T * R * S;
}

void Transform::draw(const glm::mat4& parentTransform) {
    // Transform nodes don't render geometry, they just apply transformations
    // Pass the combined transformation to children
    glm::mat4 globalTransform = parentTransform * localTransform;
    for (auto& child : children) {
        child->draw(globalTransform);
    }
}
