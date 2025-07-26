#pragma once
#include "node.hpp"
#include <glm/glm.hpp>

// Plane geometry node (for ground)
class Plane : public Node {
public:
    glm::vec3 color{0.8f, 0.8f, 0.8f};
    float size = 10.0f;
    Plane(const std::string& name_) : Node(name_) {}
    void draw(const glm::mat4& parentTransform) override;
};
