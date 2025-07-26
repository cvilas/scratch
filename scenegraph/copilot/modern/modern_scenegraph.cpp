#include "modern_scenegraph.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

ModernSceneGraph::ModernSceneGraph() {
    // Reserve space for efficient allocation
    local_transforms_.reserve(1024);
    world_transforms_.reserve(1024);
    parent_indices_.reserve(1024);
    first_child_.reserve(1024);
    next_sibling_.reserve(1024);
    node_types_.reserve(1024);
    node_flags_.reserve(1024);
    names_.reserve(1024);
    
    // Create root node
    root_node_id_ = createNode("root", NodeType::Root);
}

uint32_t ModernSceneGraph::createNode(const std::string& name, NodeType type) {
    uint32_t node_id = allocateNode();
    
    // Initialize node data
    if (node_id >= local_transforms_.size()) {
        local_transforms_.resize(node_id + 1, glm::mat4(1.0f));
        world_transforms_.resize(node_id + 1, glm::mat4(1.0f));
        parent_indices_.resize(node_id + 1, INVALID_NODE_ID);
        first_child_.resize(node_id + 1, INVALID_NODE_ID);
        next_sibling_.resize(node_id + 1, INVALID_NODE_ID);
        node_types_.resize(node_id + 1);
        node_flags_.resize(node_id + 1);
        names_.resize(node_id + 1);
    }
    
    names_[node_id] = name;
    node_types_[node_id] = type;
    node_flags_[node_id] = NodeFlags::Visible | NodeFlags::Dirty;
    
    render_lists_dirty_ = true;
    return node_id;
}

void ModernSceneGraph::destroyNode(uint32_t node_id) {
    if (!isValidNode(node_id)) return;
    
    // Remove from parent's children list
    uint32_t parent_id = parent_indices_[node_id];
    if (parent_id != INVALID_NODE_ID) {
        removeChild(parent_id, node_id);
    }
    
    // Recursively destroy children
    std::vector<uint32_t> children = getChildren(node_id);
    for (uint32_t child_id : children) {
        destroyNode(child_id);
    }
    
    // Remove components
    geometry_components_.erase(node_id);
    light_components_.erase(node_id);
    camera_components_.erase(node_id);
    separator_components_.erase(node_id);
    
    // Mark as free
    free_nodes_.push_back(node_id);
    render_lists_dirty_ = true;
}

void ModernSceneGraph::addChild(uint32_t parent_id, uint32_t child_id) {
    if (!isValidNode(parent_id) || !isValidNode(child_id)) return;
    
    // Remove child from current parent
    uint32_t old_parent = parent_indices_[child_id];
    if (old_parent != INVALID_NODE_ID) {
        removeChild(old_parent, child_id);
    }
    
    // Set new parent
    parent_indices_[child_id] = parent_id;
    
    // Add to parent's children list
    next_sibling_[child_id] = first_child_[parent_id];
    first_child_[parent_id] = child_id;
    
    // Mark transforms as dirty
    addNodeFlags(child_id, NodeFlags::Dirty);
}

void ModernSceneGraph::removeChild(uint32_t parent_id, uint32_t child_id) {
    if (!isValidNode(parent_id) || !isValidNode(child_id)) return;
    
    uint32_t* current = &first_child_[parent_id];
    while (*current != INVALID_NODE_ID) {
        if (*current == child_id) {
            *current = next_sibling_[child_id];
            next_sibling_[child_id] = INVALID_NODE_ID;
            parent_indices_[child_id] = INVALID_NODE_ID;
            break;
        }
        current = &next_sibling_[*current];
    }
}

uint32_t ModernSceneGraph::allocateNode() {
    if (!free_nodes_.empty()) {
        uint32_t node_id = free_nodes_.back();
        free_nodes_.pop_back();
        return node_id;
    }
    return next_node_id_++;
}

void ModernSceneGraph::setLocalTransform(uint32_t node_id, const glm::mat4& transform) {
    if (!isValidNode(node_id)) return;
    local_transforms_[node_id] = transform;
    addNodeFlags(node_id, NodeFlags::Dirty);
}

void ModernSceneGraph::setTranslation(uint32_t node_id, const glm::vec3& translation) {
    if (!isValidNode(node_id)) return;
    local_transforms_[node_id] = glm::translate(glm::mat4(1.0f), translation);
    addNodeFlags(node_id, NodeFlags::Dirty);
}

void ModernSceneGraph::setRotation(uint32_t node_id, const glm::vec3& rotation_degrees) {
    if (!isValidNode(node_id)) return;
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::rotate(transform, glm::radians(rotation_degrees.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(rotation_degrees.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rotation_degrees.z), glm::vec3(0, 0, 1));
    local_transforms_[node_id] = transform;
    addNodeFlags(node_id, NodeFlags::Dirty);
}

void ModernSceneGraph::setScale(uint32_t node_id, float scale) {
    setScale(node_id, glm::vec3(scale));
}

void ModernSceneGraph::setScale(uint32_t node_id, const glm::vec3& scale) {
    if (!isValidNode(node_id)) return;
    local_transforms_[node_id] = glm::scale(glm::mat4(1.0f), scale);
    addNodeFlags(node_id, NodeFlags::Dirty);
}

const glm::mat4& ModernSceneGraph::getLocalTransform(uint32_t node_id) const {
    assert(isValidNode(node_id));
    return local_transforms_[node_id];
}

const glm::mat4& ModernSceneGraph::getWorldTransform(uint32_t node_id) const {
    assert(isValidNode(node_id));
    return world_transforms_[node_id];
}

void ModernSceneGraph::addGeometryComponent(uint32_t node_id, const GeometryComponent& component) {
    if (!isValidNode(node_id)) return;
    geometry_components_[node_id] = component;
    setNodeFlags(node_id, getNodeFlags(node_id) | NodeFlags::Visible);
    render_lists_dirty_ = true;
}

void ModernSceneGraph::addLightComponent(uint32_t node_id, const LightComponent& component) {
    if (!isValidNode(node_id)) return;
    light_components_[node_id] = component;
    render_lists_dirty_ = true;
}

void ModernSceneGraph::addCameraComponent(uint32_t node_id, const CameraComponent& component) {
    if (!isValidNode(node_id)) return;
    camera_components_[node_id] = component;
}

void ModernSceneGraph::addSeparatorComponent(uint32_t node_id, const SeparatorComponent& component) {
    if (!isValidNode(node_id)) return;
    separator_components_[node_id] = component;
}

GeometryComponent* ModernSceneGraph::getGeometryComponent(uint32_t node_id) {
    auto it = geometry_components_.find(node_id);
    return (it != geometry_components_.end()) ? &it->second : nullptr;
}

LightComponent* ModernSceneGraph::getLightComponent(uint32_t node_id) {
    auto it = light_components_.find(node_id);
    return (it != light_components_.end()) ? &it->second : nullptr;
}

CameraComponent* ModernSceneGraph::getCameraComponent(uint32_t node_id) {
    auto it = camera_components_.find(node_id);
    return (it != camera_components_.end()) ? &it->second : nullptr;
}

SeparatorComponent* ModernSceneGraph::getSeparatorComponent(uint32_t node_id) {
    auto it = separator_components_.find(node_id);
    return (it != separator_components_.end()) ? &it->second : nullptr;
}

const std::string& ModernSceneGraph::getName(uint32_t node_id) const {
    assert(isValidNode(node_id));
    return names_[node_id];
}

NodeType ModernSceneGraph::getNodeType(uint32_t node_id) const {
    assert(isValidNode(node_id));
    return node_types_[node_id];
}

NodeFlags ModernSceneGraph::getNodeFlags(uint32_t node_id) const {
    assert(isValidNode(node_id));
    return node_flags_[node_id];
}

void ModernSceneGraph::setNodeFlags(uint32_t node_id, NodeFlags flags) {
    if (!isValidNode(node_id)) return;
    node_flags_[node_id] = flags;
}

void ModernSceneGraph::addNodeFlags(uint32_t node_id, NodeFlags flags) {
    if (!isValidNode(node_id)) return;
    node_flags_[node_id] = node_flags_[node_id] | flags;
}

void ModernSceneGraph::removeNodeFlags(uint32_t node_id, NodeFlags flags) {
    if (!isValidNode(node_id)) return;
    node_flags_[node_id] = static_cast<NodeFlags>(static_cast<uint32_t>(node_flags_[node_id]) & ~static_cast<uint32_t>(flags));
}

bool ModernSceneGraph::isValidNode(uint32_t node_id) const {
    return node_id < local_transforms_.size() && 
           std::find(free_nodes_.begin(), free_nodes_.end(), node_id) == free_nodes_.end();
}

std::vector<uint32_t> ModernSceneGraph::getChildren(uint32_t node_id) const {
    std::vector<uint32_t> children;
    if (!isValidNode(node_id)) return children;
    
    uint32_t child_id = first_child_[node_id];
    while (child_id != INVALID_NODE_ID) {
        children.push_back(child_id);
        child_id = next_sibling_[child_id];
    }
    return children;
}

uint32_t ModernSceneGraph::getParent(uint32_t node_id) const {
    if (!isValidNode(node_id)) return INVALID_NODE_ID;
    return parent_indices_[node_id];
}

void ModernSceneGraph::updateTransforms() {
    if (root_node_id_ != INVALID_NODE_ID) {
        updateWorldTransform(root_node_id_, glm::mat4(1.0f));
    }
}

void ModernSceneGraph::updateWorldTransform(uint32_t node_id, const glm::mat4& parent_transform) {
    if (!isValidNode(node_id)) return;
    
    // Update world transform
    world_transforms_[node_id] = parent_transform * local_transforms_[node_id];
    
    // Clear dirty flag
    removeNodeFlags(node_id, NodeFlags::Dirty);
    
    // Update children
    uint32_t child_id = first_child_[node_id];
    while (child_id != INVALID_NODE_ID) {
        updateWorldTransform(child_id, world_transforms_[node_id]);
        child_id = next_sibling_[child_id];
    }
}

void ModernSceneGraph::updateRenderLists() {
    if (!render_lists_dirty_) return;
    
    geometry_nodes_.clear();
    light_nodes_.clear();
    
    for (uint32_t i = 0; i < local_transforms_.size(); ++i) {
        if (!isValidNode(i)) continue;
        
        if (geometry_components_.find(i) != geometry_components_.end()) {
            geometry_nodes_.push_back(i);
        }
        if (light_components_.find(i) != light_components_.end()) {
            light_nodes_.push_back(i);
        }
    }
    
    render_lists_dirty_ = false;
}

void ModernSceneGraph::draw() {
    updateTransforms();
    updateRenderLists();
    
    // Setup lights first
    int light_index = 0;
    for (uint32_t node_id : light_nodes_) {
        if (light_index >= 8) break; // OpenGL limit
        auto* light = getLightComponent(node_id);
        if (light && hasFlag(getNodeFlags(node_id), NodeFlags::Visible)) {
            glEnable(GL_LIGHT0 + light_index);
            setupLight(node_id, *light);
            light_index++;
        }
    }
    
    // Draw geometry nodes
    for (uint32_t node_id : geometry_nodes_) {
        if (hasFlag(getNodeFlags(node_id), NodeFlags::Visible)) {
            drawNode(node_id);
        }
    }
}

void ModernSceneGraph::drawNode(uint32_t node_id) {
    auto* geometry = getGeometryComponent(node_id);
    auto* separator = getSeparatorComponent(node_id);
    
    // Handle separator state saving
    bool pushed_matrix = false;
    GLfloat saved_material[4];
    GLfloat saved_color[4];
    
    if (separator) {
        if (separator->saveTransform) {
            glPushMatrix();
            pushed_matrix = true;
        }
        if (separator->saveMaterial) {
            glGetMaterialfv(GL_FRONT, GL_DIFFUSE, saved_material);
        }
        if (separator->saveColor) {
            glGetFloatv(GL_CURRENT_COLOR, saved_color);
        }
    }
    
    // Apply world transform (not local transform, since world is already computed)
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(getWorldTransform(node_id)));
    
    // Draw geometry if present
    if (geometry) {
        drawGeometry(node_id, *geometry);
    }
    
    glPopMatrix();
    
    // Restore separator state
    if (separator) {
        if (separator->saveColor) {
            glColor4fv(saved_color);
        }
        if (separator->saveMaterial) {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, saved_material);
        }
        if (pushed_matrix) {
            glPopMatrix();
        }
    }
}

void ModernSceneGraph::drawGeometry(uint32_t node_id, const GeometryComponent& geometry) {
    glColor3f(geometry.color.r, geometry.color.g, geometry.color.b);
    
    switch (geometry.type) {
        case GeometryType::Cube:
            renderCube(geometry);
            break;
        case GeometryType::Sphere:
            renderSphere(geometry);
            break;
        case GeometryType::Cylinder:
            renderCylinder(geometry);
            break;
        case GeometryType::Cone:
            renderCone(geometry);
            break;
        case GeometryType::Plane:
            renderPlane(geometry);
            break;
    }
}

void ModernSceneGraph::setupLight(uint32_t node_id, const LightComponent& light) {
    // This would need to be implemented based on your lighting system
    // Similar to your existing Light class implementation
}

void ModernSceneGraph::printHierarchy() const {
    std::cout << "\n=== Modern Scene Graph Hierarchy ===\n";
    if (root_node_id_ != INVALID_NODE_ID) {
        printNodeHierarchy(root_node_id_, 0);
    }
    std::cout << "====================================\n\n";
}

void ModernSceneGraph::printNodeHierarchy(uint32_t node_id, int depth) const {
    if (!isValidNode(node_id)) return;
    
    std::string indent(depth * 2, ' ');
    std::string nodeInfo = names_[node_id];
    
    // Add node type and component info
    switch (node_types_[node_id]) {
        case NodeType::Transform: nodeInfo += " [Transform]"; break;
        case NodeType::Separator: nodeInfo += " [Separator]"; break;
        case NodeType::Geometry: nodeInfo += " [Geometry]"; break;
        case NodeType::Camera: nodeInfo += " [Camera]"; break;
        case NodeType::Light: nodeInfo += " [Light]"; break;
        case NodeType::Root: break;
    }
    
    std::cout << indent << "- " << nodeInfo << std::endl;
    
    // Print children
    uint32_t child_id = first_child_[node_id];
    while (child_id != INVALID_NODE_ID) {
        printNodeHierarchy(child_id, depth + 1);
        child_id = next_sibling_[child_id];
    }
}

// Geometry rendering implementations
void ModernSceneGraph::renderCube(const GeometryComponent& geometry) {
    float size = geometry.params.cube.size;
    float half = size * 0.5f;
    
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0, 0, 1);
    glVertex3f(-half, -half, half);
    glVertex3f(half, -half, half);
    glVertex3f(half, half, half);
    glVertex3f(-half, half, half);
    
    // Back face
    glNormal3f(0, 0, -1);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);
    
    // Top face
    glNormal3f(0, 1, 0);
    glVertex3f(-half, half, -half);
    glVertex3f(-half, half, half);
    glVertex3f(half, half, half);
    glVertex3f(half, half, -half);
    
    // Bottom face
    glNormal3f(0, -1, 0);
    glVertex3f(-half, -half, -half);
    glVertex3f(half, -half, -half);
    glVertex3f(half, -half, half);
    glVertex3f(-half, -half, half);
    
    // Right face
    glNormal3f(1, 0, 0);
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half, half);
    glVertex3f(half, -half, half);
    
    // Left face
    glNormal3f(-1, 0, 0);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half, half, -half);
    glEnd();
}

void ModernSceneGraph::renderSphere(const GeometryComponent& geometry) {
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, geometry.params.sphere.radius, geometry.params.sphere.slices, geometry.params.sphere.stacks);
    gluDeleteQuadric(quad);
}

void ModernSceneGraph::renderCylinder(const GeometryComponent& geometry) {
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, geometry.params.cylinder.radius, geometry.params.cylinder.radius, geometry.params.cylinder.height, 16, 1);
    gluDeleteQuadric(quad);
}

void ModernSceneGraph::renderCone(const GeometryComponent& geometry) {
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, geometry.params.cone.baseRadius, 0.0, geometry.params.cone.height, geometry.params.cone.slices, 1);
    gluDeleteQuadric(quad);
}

void ModernSceneGraph::renderPlane(const GeometryComponent& geometry) {
    float w = geometry.params.plane.width * 0.5f;
    float h = geometry.params.plane.height * 0.5f;
    
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-w, 0, -h);
    glVertex3f(w, 0, -h);
    glVertex3f(w, 0, h);
    glVertex3f(-w, 0, h);
    glEnd();
}

// Helper functions
uint32_t createTransformNode(ModernSceneGraph& graph, const std::string& name) {
    return graph.createNode(name, NodeType::Transform);
}

uint32_t createSeparatorNode(ModernSceneGraph& graph, const std::string& name, bool saveTransform, bool saveMaterial, bool saveColor) {
    uint32_t node_id = graph.createNode(name, NodeType::Separator);
    SeparatorComponent separator;
    separator.saveTransform = saveTransform;
    separator.saveMaterial = saveMaterial;
    separator.saveColor = saveColor;
    graph.addSeparatorComponent(node_id, separator);
    return node_id;
}

uint32_t createGeometryNode(ModernSceneGraph& graph, const std::string& name, GeometryType type) {
    return graph.createNode(name, NodeType::Geometry);
}

uint32_t createLightNode(ModernSceneGraph& graph, const std::string& name) {
    return graph.createNode(name, NodeType::Light);
}

uint32_t createCameraNode(ModernSceneGraph& graph, const std::string& name) {
    return graph.createNode(name, NodeType::Camera);
}

// Geometry creation helpers
GeometryComponent createCubeGeometry(float size, const glm::vec3& color) {
    GeometryComponent geometry;
    geometry.type = GeometryType::Cube;
    geometry.color = color;
    geometry.params.cube.size = size;
    return geometry;
}

GeometryComponent createSphereGeometry(float radius, int slices, int stacks, const glm::vec3& color) {
    GeometryComponent geometry;
    geometry.type = GeometryType::Sphere;
    geometry.color = color;
    geometry.params.sphere.radius = radius;
    geometry.params.sphere.slices = slices;
    geometry.params.sphere.stacks = stacks;
    return geometry;
}

GeometryComponent createCylinderGeometry(float radius, float height, const glm::vec3& color) {
    GeometryComponent geometry;
    geometry.type = GeometryType::Cylinder;
    geometry.color = color;
    geometry.params.cylinder.radius = radius;
    geometry.params.cylinder.height = height;
    return geometry;
}

GeometryComponent createConeGeometry(float baseRadius, float height, int slices, const glm::vec3& color) {
    GeometryComponent geometry;
    geometry.type = GeometryType::Cone;
    geometry.color = color;
    geometry.params.cone.baseRadius = baseRadius;
    geometry.params.cone.height = height;
    geometry.params.cone.slices = slices;
    return geometry;
}

GeometryComponent createPlaneGeometry(float width, float height, const glm::vec3& color) {
    GeometryComponent geometry;
    geometry.type = GeometryType::Plane;
    geometry.color = color;
    geometry.params.plane.width = width;
    geometry.params.plane.height = height;
    return geometry;
}
