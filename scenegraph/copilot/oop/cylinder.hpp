#pragma once
#include "node.hpp"
#include <glm/glm.hpp>

// Cylinder geometry node
class Cylinder : public Node {
public:
    glm::vec3 color{0.3f, 0.7f, 0.9f};
    float radius = 0.5f;
    float height = 1.0f;
    int slices = 24;
    Cylinder(const std::string& name_) : Node(name_) {}
    void draw(const glm::mat4& parentTransform) override;
};
