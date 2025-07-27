#pragma once
#include "node.hpp"


// Separator node - provides isolation and grouping
// Saves and restores OpenGL state, useful for grouping objects
class Separator : public Node {
public:
    // Options for what state to save/restore
    bool saveTransform = true;      // Save/restore matrix stack
    bool saveMaterial = false;      // Save/restore material properties
    bool saveColor = false;         // Save/restore current color
    bool saveTexture = false;       // Save/restore texture state
    
    Separator(const std::string& name_);
    
    // Override draw to provide state isolation
    void draw(const glm::mat4& parentTransform) override;
    
    // Convenience methods for state management
    void enableAllSaving();
    void disableAllSaving();
    void enableMaterialSaving();
    void enableColorSaving();
};
