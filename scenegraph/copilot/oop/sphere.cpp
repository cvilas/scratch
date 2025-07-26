#include "sphere.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <GL/gl.h>

// Sphere rendering
void Sphere::draw(const glm::mat4& parentTransform) {
    // Set material properties for lighting while preserving colors
    GLfloat matDiffuse[] = {color.r, color.g, color.b, 1.0f};
    GLfloat matSpecular[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat matShininess[] = {32.0f};
    GLfloat matAmbient[] = {color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, 1.0f};
    
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    
    glColor3f(color.r, color.g, color.b);
    
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(parentTransform * localTransform));

    // Draw sphere using triangle strips
    for (int i = 0; i < stacks; ++i) {
        float lat1 = M_PI * (i / (float)stacks - 0.5f);
        float lat2 = M_PI * ((i + 1) / (float)stacks - 0.5f);
        
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lon = 2 * M_PI * j / (float)slices;
            
            // First vertex
            float x1 = radius * cos(lat1) * cos(lon);
            float y1 = radius * sin(lat1);
            float z1 = radius * cos(lat1) * sin(lon);
            glm::vec3 normal1 = glm::normalize(glm::vec3(x1, y1, z1));
            glNormal3f(normal1.x, normal1.y, normal1.z);
            glVertex3f(x1, y1, z1);
            
            // Second vertex
            float x2 = radius * cos(lat2) * cos(lon);
            float y2 = radius * sin(lat2);
            float z2 = radius * cos(lat2) * sin(lon);
            glm::vec3 normal2 = glm::normalize(glm::vec3(x2, y2, z2));
            glNormal3f(normal2.x, normal2.y, normal2.z);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }

    glPopMatrix();

    // Draw children
    for (auto& child : children)
        child->draw(parentTransform * localTransform);
}
