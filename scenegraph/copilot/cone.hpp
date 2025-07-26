#pragma once
#include "node.hpp"
#include <glm/glm.hpp>

// Cone geometry node
class Cone : public Node {
public:
    glm::vec3 color{1,0.5,0};
    Cone(const std::string& name_) : Node(name_) {}
    void draw(const glm::mat4& parentTransform) override;
};
