#include "cone.hpp" 
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <GL/gl.h>

// Simple cone rendering (using immediate mode for demonstration)
void Cone::draw(const glm::mat4& parentTransform) {
    // Set material properties for lighting while preserving colors
    GLfloat matDiffuse[] = {color.r, color.g, color.b, 1.0f};
    GLfloat matSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};  // Reduced specular to preserve colors
    GLfloat matShininess[] = {16.0f};  // Reduced shininess
    GLfloat matAmbient[] = {color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, 1.0f}; // Keep object colors in ambient
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    
    // Also set the color directly for better color preservation
    glColor3f(color.r, color.g, color.b);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(parentTransform * localTransform));

    // Draw cone with proper normals for lighting
    const int slices = 24;
    const float radius = 0.5f;
    const float height = 1.0f;
    
    // Draw cone body (sides)
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < slices; ++i) {
        float theta1 = i * 2.0f * M_PI / slices;
        float theta2 = (i + 1) * 2.0f * M_PI / slices;
        
        // Base vertices
        glm::vec3 v1(radius * cos(theta1), radius * sin(theta1), 0);
        glm::vec3 v2(radius * cos(theta2), radius * sin(theta2), 0);
        glm::vec3 v3(0, 0, height); // Apex
        
        // Calculate normal for this triangle (ensure correct winding)
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        // Render triangle with normals (counter-clockwise winding)
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
    
    // Draw cone base (visible from below)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1); // Base faces downward
    glVertex3f(0, 0, 0);  // Center of base
    for (int i = slices; i >= 0; --i) { // Reverse order for correct winding when viewed from below
        float theta = i * 2.0f * M_PI / slices;
        glVertex3f(radius * cos(theta), radius * sin(theta), 0);
    }
    glEnd();

    glPopMatrix();

    // Draw children
    for (auto& child : children)
        child->draw(parentTransform * localTransform);
}
