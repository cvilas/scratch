#pragma once
#include "node.hpp"
#include <glm/glm.hpp>

// Light node
class Light : public Node {
public:
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    float intensity = 1.0f;
    
    // Spotlight parameters
    float cutoffAngle = 30.0f;    // Inner cone angle in degrees
    float outerCutoffAngle = 45.0f; // Outer cone angle in degrees
    float constant = 1.0f;        // Attenuation constant
    float linear = 0.09f;         // Attenuation linear
    float quadratic = 0.032f;     // Attenuation quadratic
    
    Light(const std::string& name_);
    
    void setupSpotlight(const glm::vec3& position, const glm::vec3& direction);
    void setupMovingSpotlight(const glm::vec3& position, const glm::vec3& direction, int lightIndex);
    
    // Static method to set up global ambient lighting
    static void setupGlobalAmbient(float intensity = 0.4f);
};
