#include "light.hpp"
#include <GL/gl.h>

Light::Light(const std::string& name_) : Node(name_) {}

void Light::setupSpotlight(const glm::vec3& position, const glm::vec3& direction) {
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set global ambient light (reduced to preserve colors)
    GLfloat globalAmbient[] = {0.25f, 0.25f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    // Set light position (w=1 for positional light)
    GLfloat lightPos[] = {position.x, position.y, position.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    
    // Set light direction
    GLfloat lightDir[] = {direction.x, direction.y, direction.z};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDir);
    
    // Set light colors (brighter spotlight to compensate)
    GLfloat lightColor[] = {color.r * intensity * 1.5f, color.g * intensity * 1.5f, color.b * intensity * 1.5f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    
    // Set per-light ambient (reduced)
    GLfloat ambientColor[] = {color.r * 0.2f, color.g * 0.2f, color.b * 0.2f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
    
    // Spotlight parameters
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, outerCutoffAngle);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 32.0f); // Focus of the spotlight
    
    // Attenuation
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic);
}

// Static method to set up global ambient lighting
void Light::setupGlobalAmbient(float intensity) {
    GLfloat globalAmbient[] = {intensity, intensity, intensity, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
}

// Setup a moving spotlight from above
void Light::setupMovingSpotlight(const glm::vec3& position, const glm::vec3& direction, int lightIndex) {
    glEnable(GL_LIGHTING);
    glEnable(lightIndex);
    
    // Set light position (w=1 for positional light)
    GLfloat lightPos[] = {position.x, position.y, position.z, 1.0f};
    glLightfv(lightIndex, GL_POSITION, lightPos);
    
    // Set light direction
    GLfloat lightDir[] = {direction.x, direction.y, direction.z};
    glLightfv(lightIndex, GL_SPOT_DIRECTION, lightDir);
    
    // Set light colors (different color for moving light)
    GLfloat lightColor[] = {color.r * intensity * 0.8f, color.g * intensity * 0.8f, color.b * intensity * 0.8f, 1.0f};
    glLightfv(lightIndex, GL_DIFFUSE, lightColor);
    glLightfv(lightIndex, GL_SPECULAR, lightColor);
    
    // Set per-light ambient
    GLfloat ambientColor[] = {color.r * 0.1f, color.g * 0.1f, color.b * 0.1f, 1.0f};
    glLightfv(lightIndex, GL_AMBIENT, ambientColor);
    
    // Spotlight parameters (wider cone for area coverage)
    glLightf(lightIndex, GL_SPOT_CUTOFF, 60.0f); // Wider angle
    glLightf(lightIndex, GL_SPOT_EXPONENT, 16.0f); // Softer focus
    
    // Attenuation
    glLightf(lightIndex, GL_CONSTANT_ATTENUATION, constant);
    glLightf(lightIndex, GL_LINEAR_ATTENUATION, linear * 0.5f); // Less attenuation
    glLightf(lightIndex, GL_QUADRATIC_ATTENUATION, quadratic * 0.25f);
}
