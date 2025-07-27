#include "separator.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>

Separator::Separator(const std::string& name_) : Node(name_) {}

void Separator::draw(const glm::mat4& parentTransform) {
    // Save OpenGL state before drawing children
    if (saveTransform) {
        glPushMatrix();
    }
    
    // Save material properties if requested
    GLfloat savedMaterial[4];
    if (saveMaterial) {
        glGetMaterialfv(GL_FRONT, GL_DIFFUSE, savedMaterial);
    }
    
    // Save color if requested
    GLfloat savedColor[4];
    if (saveColor) {
        glGetFloatv(GL_CURRENT_COLOR, savedColor);
    }
    
    // Apply this node's local transformation and draw children
    glm::mat4 globalTransform = parentTransform * localTransform;
    
    if (saveTransform) {
        // Apply the transformation to OpenGL matrix stack
        glMultMatrixf(glm::value_ptr(localTransform));
        
        // Draw children with identity parent transform since we've applied it to GL stack
        for (auto& child : children) {
            child->draw(glm::mat4(1.0f));
        }
    } else {
        // Draw children normally with combined transform
        for (auto& child : children) {
            child->draw(globalTransform);
        }
    }
    
    // Restore OpenGL state after drawing children
    if (saveColor) {
        glColor4fv(savedColor);
    }
    
    if (saveMaterial) {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, savedMaterial);
    }
    
    if (saveTransform) {
        glPopMatrix();
    }
}

void Separator::enableAllSaving() {
    saveTransform = true;
    saveMaterial = true;
    saveColor = true;
    saveTexture = true;
}

void Separator::disableAllSaving() {
    saveTransform = false;
    saveMaterial = false;
    saveColor = false;
    saveTexture = false;
}

void Separator::enableMaterialSaving() {
    saveMaterial = true;
}

void Separator::enableColorSaving() {
    saveColor = true;
}
