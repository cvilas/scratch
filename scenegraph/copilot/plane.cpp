#include "plane.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>

// Simple plane rendering for ground
void Plane::draw(const glm::mat4& parentTransform) {
    // Set material properties for lighting while preserving visibility
    GLfloat matDiffuse[] = {color.r, color.g, color.b, 1.0f};
    GLfloat matSpecular[] = {0.1f, 0.1f, 0.1f, 1.0f}; // Low specular for ground
    GLfloat matShininess[] = {4.0f}; // Low shininess
    GLfloat matAmbient[] = {color.r * 0.9f, color.g * 0.9f, color.b * 0.9f, 1.0f}; // High ambient for visibility
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(parentTransform * localTransform));

    // Draw a textured ground plane
    const int divisions = 20;
    const float step = size / divisions;
    
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0); // Normal pointing up
    
    for (int i = 0; i < divisions; ++i) {
        for (int j = 0; j < divisions; ++j) {
            float x1 = -size/2 + i * step;
            float x2 = -size/2 + (i+1) * step;
            float z1 = -size/2 + j * step;
            float z2 = -size/2 + (j+1) * step;
            
            // Create a checkerboard pattern
            float brightness = ((i + j) % 2 == 0) ? 1.0f : 0.7f;
            glColor3f(color.r * brightness, color.g * brightness, color.b * brightness);
            
            // Vertices in counter-clockwise order when viewed from above (correct winding)
            glVertex3f(x1, 0, z1);
            glVertex3f(x1, 0, z2);
            glVertex3f(x2, 0, z2);
            glVertex3f(x2, 0, z1);
        }
    }
    glEnd();

    glPopMatrix();

    // Draw children
    for (auto& child : children)
        child->draw(parentTransform * localTransform);
}
