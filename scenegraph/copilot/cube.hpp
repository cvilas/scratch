#pragma once
#include "node.hpp"
#include <glm/glm.hpp>

// Cube geometry node
class Cube : public Node {
public:
    glm::vec3 color{0.8f, 0.3f, 0.8f};
    float size = 1.0f;
    Cube(const std::string& name_) : Node(name_) {}
    void draw(const glm::mat4& parentTransform) override;
};
