#include "cylinder.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <GL/gl.h>

// Cylinder rendering
void Cylinder::draw(const glm::mat4& parentTransform) {
    // Set material properties for lighting while preserving colors
    GLfloat matDiffuse[] = {color.r, color.g, color.b, 1.0f};
    GLfloat matSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat matShininess[] = {16.0f};
    GLfloat matAmbient[] = {color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, 1.0f};
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    
    glColor3f(color.r, color.g, color.b);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(parentTransform * localTransform));

    // Draw cylinder sides
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float theta = i * 2.0f * M_PI / slices;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // Normal points outward
        glNormal3f(cos(theta), 0, sin(theta));
        
        // Bottom vertex
        glVertex3f(x, 0, z);
        // Top vertex
        glVertex3f(x, height, z);
    }
    glEnd();
    
    // Draw bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, 0, 0);
    for (int i = slices; i >= 0; --i) {
        float theta = i * 2.0f * M_PI / slices;
        glVertex3f(radius * cos(theta), 0, radius * sin(theta));
    }
    glEnd();
    
    // Draw top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, height, 0);
    for (int i = 0; i <= slices; ++i) {
        float theta = i * 2.0f * M_PI / slices;
        glVertex3f(radius * cos(theta), height, radius * sin(theta));
    }
    glEnd();

    glPopMatrix();

    // Draw children
    for (auto& child : children)
        child->draw(parentTransform * localTransform);
}
