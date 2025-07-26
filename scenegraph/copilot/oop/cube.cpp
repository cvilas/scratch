#include "cube.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>

// Cube rendering
void Cube::draw(const glm::mat4& parentTransform) {
    // Set material properties for lighting while preserving colors
    GLfloat matDiffuse[] = {color.r, color.g, color.b, 1.0f};
    GLfloat matSpecular[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat matShininess[] = {8.0f};
    GLfloat matAmbient[] = {color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, 1.0f};
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    
    glColor3f(color.r, color.g, color.b);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(parentTransform * localTransform));

    float halfSize = size * 0.5f;
    
    // Define cube faces with proper normals and counter-clockwise winding
    
    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-halfSize, -halfSize,  halfSize);
    glVertex3f( halfSize, -halfSize,  halfSize);
    glVertex3f( halfSize,  halfSize,  halfSize);
    glVertex3f(-halfSize,  halfSize,  halfSize);
    glEnd();
    
    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize,  halfSize, -halfSize);
    glVertex3f( halfSize,  halfSize, -halfSize);
    glVertex3f( halfSize, -halfSize, -halfSize);
    glEnd();
    
    // Top face
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-halfSize,  halfSize, -halfSize);
    glVertex3f(-halfSize,  halfSize,  halfSize);
    glVertex3f( halfSize,  halfSize,  halfSize);
    glVertex3f( halfSize,  halfSize, -halfSize);
    glEnd();
    
    // Bottom face
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f( halfSize, -halfSize, -halfSize);
    glVertex3f( halfSize, -halfSize,  halfSize);
    glVertex3f(-halfSize, -halfSize,  halfSize);
    glEnd();
    
    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glVertex3f( halfSize, -halfSize, -halfSize);
    glVertex3f( halfSize,  halfSize, -halfSize);
    glVertex3f( halfSize,  halfSize,  halfSize);
    glVertex3f( halfSize, -halfSize,  halfSize);
    glEnd();
    
    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize,  halfSize);
    glVertex3f(-halfSize,  halfSize,  halfSize);
    glVertex3f(-halfSize,  halfSize, -halfSize);
    glEnd();

    glPopMatrix();

    // Draw children
    for (auto& child : children)
        child->draw(parentTransform * localTransform);
}
