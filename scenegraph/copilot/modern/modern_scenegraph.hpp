#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <memory>
#include <functional>

// Forward declarations
class Camera;
class Light;

// Node types for efficient dispatching
enum class NodeType : uint8_t {
    Transform,
    Separator,
    Geometry,
    Camera,
    Light,
    Root
};

// Geometry types
enum class GeometryType : uint8_t {
    Cube,
    Sphere,
    Cylinder,
    Cone,
    Plane
};

// Node flags for efficient filtering
enum class NodeFlags : uint32_t {
    None = 0,
    Visible = 1 << 0,
    CastsShadows = 1 << 1,
    ReceivesShadows = 1 << 2,
    SaveTransform = 1 << 3,
    SaveMaterial = 1 << 4,
    SaveColor = 1 << 5,
    Dirty = 1 << 6  // Transform needs update
};

inline NodeFlags operator|(NodeFlags a, NodeFlags b) {
    return static_cast<NodeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline NodeFlags operator&(NodeFlags a, NodeFlags b) {
    return static_cast<NodeFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool hasFlag(NodeFlags flags, NodeFlags flag) {
    return (flags & flag) != NodeFlags::None;
}

// Component structures (Plain Old Data for cache efficiency)
struct GeometryComponent {
    GeometryType type;
    glm::vec3 color{1.0f};
    union {
        struct { float radius, height; } cylinder;
        struct { float radius; int slices, stacks; } sphere;
        struct { float size; } cube;
        struct { float baseRadius, height; int slices; } cone;
        struct { float width, height; } plane;
    } params;
};

struct LightComponent {
    glm::vec3 color{1.0f};
    float intensity{1.0f};
    float cutoffAngle{30.0f};
    float outerCutoffAngle{35.0f};
};

struct CameraComponent {
    float fov{45.0f};
    float nearPlane{0.1f};
    float farPlane{100.0f};
    float yaw{-90.0f};
    float pitch{0.0f};
    float radius{5.0f};
    glm::vec3 target{0.0f};
};

// Separator state
struct SeparatorComponent {
    bool saveTransform{true};
    bool saveMaterial{false};
    bool saveColor{false};
    bool saveTexture{false};
};

// Invalid node ID constant
constexpr uint32_t INVALID_NODE_ID = 0xFFFFFFFF;

// Modern flat scene graph implementation
class ModernSceneGraph {
private:
    // Structure of Arrays (SoA) for cache efficiency
    // Hot data (accessed every frame during traversal)
    std::vector<glm::mat4> local_transforms_;
    std::vector<glm::mat4> world_transforms_;
    std::vector<uint32_t> parent_indices_;
    std::vector<uint32_t> first_child_;
    std::vector<uint32_t> next_sibling_;
    std::vector<NodeType> node_types_;
    std::vector<NodeFlags> node_flags_;
    
    // Cold data (less frequently accessed)
    std::vector<std::string> names_;
    
    // Component storage (sparse - only allocated when needed)
    std::unordered_map<uint32_t, GeometryComponent> geometry_components_;
    std::unordered_map<uint32_t, LightComponent> light_components_;
    std::unordered_map<uint32_t, CameraComponent> camera_components_;
    std::unordered_map<uint32_t, SeparatorComponent> separator_components_;
    
    // Node management
    std::vector<uint32_t> free_nodes_;  // Recycled node IDs
    uint32_t next_node_id_{0};
    uint32_t root_node_id_{INVALID_NODE_ID};
    
    // Cached render lists for efficiency
    std::vector<uint32_t> geometry_nodes_;
    std::vector<uint32_t> light_nodes_;
    bool render_lists_dirty_{true};

public:
    ModernSceneGraph();
    ~ModernSceneGraph() = default;
    
    // Node creation and management
    uint32_t createNode(const std::string& name, NodeType type);
    void destroyNode(uint32_t node_id);
    void addChild(uint32_t parent_id, uint32_t child_id);
    void removeChild(uint32_t parent_id, uint32_t child_id);
    
    // Root node management
    uint32_t getRootNode() const { return root_node_id_; }
    void setRootNode(uint32_t node_id) { root_node_id_ = node_id; }
    
    // Transform operations
    void setLocalTransform(uint32_t node_id, const glm::mat4& transform);
    void setTranslation(uint32_t node_id, const glm::vec3& translation);
    void setRotation(uint32_t node_id, const glm::vec3& rotation_degrees);
    void setScale(uint32_t node_id, float scale);
    void setScale(uint32_t node_id, const glm::vec3& scale);
    
    const glm::mat4& getLocalTransform(uint32_t node_id) const;
    const glm::mat4& getWorldTransform(uint32_t node_id) const;
    
    // Component operations
    void addGeometryComponent(uint32_t node_id, const GeometryComponent& component);
    void addLightComponent(uint32_t node_id, const LightComponent& component);
    void addCameraComponent(uint32_t node_id, const CameraComponent& component);
    void addSeparatorComponent(uint32_t node_id, const SeparatorComponent& component);
    
    GeometryComponent* getGeometryComponent(uint32_t node_id);
    LightComponent* getLightComponent(uint32_t node_id);
    CameraComponent* getCameraComponent(uint32_t node_id);
    SeparatorComponent* getSeparatorComponent(uint32_t node_id);
    
    // Node properties
    const std::string& getName(uint32_t node_id) const;
    NodeType getNodeType(uint32_t node_id) const;
    NodeFlags getNodeFlags(uint32_t node_id) const;
    void setNodeFlags(uint32_t node_id, NodeFlags flags);
    void addNodeFlags(uint32_t node_id, NodeFlags flags);
    void removeNodeFlags(uint32_t node_id, NodeFlags flags);
    
    // Scene operations
    void updateTransforms();
    void draw();
    void printHierarchy() const;
    
    // Utility functions
    bool isValidNode(uint32_t node_id) const;
    std::vector<uint32_t> getChildren(uint32_t node_id) const;
    uint32_t getParent(uint32_t node_id) const;

private:
    // Internal helper functions
    uint32_t allocateNode();
    void updateWorldTransform(uint32_t node_id, const glm::mat4& parent_transform);
    void updateRenderLists();
    void drawNode(uint32_t node_id);
    void drawGeometry(uint32_t node_id, const GeometryComponent& geometry);
    void setupLight(uint32_t node_id, const LightComponent& light);
    void printNodeHierarchy(uint32_t node_id, int depth) const;
    
    // Geometry rendering functions
    void renderCube(const GeometryComponent& geometry);
    void renderSphere(const GeometryComponent& geometry);
    void renderCylinder(const GeometryComponent& geometry);
    void renderCone(const GeometryComponent& geometry);
    void renderPlane(const GeometryComponent& geometry);
};

// Helper functions for creating common node types
uint32_t createTransformNode(ModernSceneGraph& graph, const std::string& name);
uint32_t createSeparatorNode(ModernSceneGraph& graph, const std::string& name, bool saveTransform = true, bool saveMaterial = false, bool saveColor = false);
uint32_t createGeometryNode(ModernSceneGraph& graph, const std::string& name, GeometryType type);
uint32_t createLightNode(ModernSceneGraph& graph, const std::string& name);
uint32_t createCameraNode(ModernSceneGraph& graph, const std::string& name);

// Geometry creation helpers
GeometryComponent createCubeGeometry(float size = 1.0f, const glm::vec3& color = glm::vec3(1.0f));
GeometryComponent createSphereGeometry(float radius = 1.0f, int slices = 16, int stacks = 8, const glm::vec3& color = glm::vec3(1.0f));
GeometryComponent createCylinderGeometry(float radius = 1.0f, float height = 2.0f, const glm::vec3& color = glm::vec3(1.0f));
GeometryComponent createConeGeometry(float baseRadius = 1.0f, float height = 2.0f, int slices = 16, const glm::vec3& color = glm::vec3(1.0f));
GeometryComponent createPlaneGeometry(float width = 2.0f, float height = 2.0f, const glm::vec3& color = glm::vec3(1.0f));
