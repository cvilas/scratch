#pragma once
#include "node.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Transform node - handles transformations without geometry
class Transform : public Node {
public:
    // Transformation components
    glm::vec3 translation{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};    // Euler angles in degrees
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    
    Transform(const std::string& name_);
    
    // Convenience methods for setting transformations
    void setTranslation(const glm::vec3& t);
    void setRotation(const glm::vec3& r);        // Euler angles in degrees
    void setScale(const glm::vec3& s);
    void setScale(float uniform_scale);
    
    // Methods for modifying transformations
    void translate(const glm::vec3& delta);
    void rotate(const glm::vec3& delta);         // Delta rotation in degrees
    void scaleBy(const glm::vec3& factor);
    void scaleBy(float factor);
    
    // Update the local transformation matrix based on components
    void updateTransform();
    
    // Override draw to apply transformation
    void draw(const glm::mat4& parentTransform) override;
};
