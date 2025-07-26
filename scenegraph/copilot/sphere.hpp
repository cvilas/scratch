#pragma once
#include "node.hpp"
#include <glm/glm.hpp>

// Sphere geometry node
class Sphere : public Node {
public:
    glm::vec3 color{1.0f, 0.6f, 0.2f};
    float radius = 0.5f;
    int slices = 20;
    int stacks = 20;
    Sphere(const std::string& name_) : Node(name_) {}
    void draw(const glm::mat4& parentTransform) override;
};
