#pragma once
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Base Node class for scene graph
class Node {
public:
    using Ptr = std::shared_ptr<Node>;
    std::string name;
    glm::mat4 localTransform{1.0f};
    std::vector<Ptr> children;

    Node(const std::string& name_) : name(name_) {}
    virtual ~Node() = default;

    void addChild(Ptr child) {
        children.push_back(child);
    }

    // Recursive draw interface
    virtual void draw(const glm::mat4& parentTransform) {
        glm::mat4 globalTransform = parentTransform * localTransform;
        for (auto& child : children)
            child->draw(globalTransform);
    }
};
