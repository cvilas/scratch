#include "modern_scenegraph.hpp"
#include <iostream>
#include <chrono>
#include <cmath>

// Example showing how to convert from the old hierarchy to the modern approach
void createModernScene(ModernSceneGraph& graph) {
    uint32_t root = graph.getRootNode();
    
    // Create camera
    uint32_t camera = createCameraNode(graph, "main_camera");
    graph.addChild(root, camera);
    
    // Create lights
    uint32_t camera_light = createLightNode(graph, "camera_spotlight");
    LightComponent camera_light_comp;
    camera_light_comp.color = glm::vec3(1.0f, 0.95f, 0.8f);
    camera_light_comp.intensity = 1.2f;
    graph.addLightComponent(camera_light, camera_light_comp);
    graph.addChild(root, camera_light);
    
    uint32_t moving_light = createLightNode(graph, "moving_spotlight");
    LightComponent moving_light_comp;
    moving_light_comp.color = glm::vec3(0.9f, 0.9f, 1.0f);
    moving_light_comp.intensity = 2.0f;
    graph.addLightComponent(moving_light, moving_light_comp);
    graph.addChild(root, moving_light);
    
    // Create ground plane
    uint32_t ground = createGeometryNode(graph, "ground", GeometryType::Plane);
    graph.addGeometryComponent(ground, createPlaneGeometry(20.0f, 20.0f, glm::vec3(0.3f, 0.3f, 0.3f)));
    graph.addChild(root, ground);
    
    // Create cone group with separator
    uint32_t cone_group = createSeparatorNode(graph, "cone_group", true, false, false);
    graph.addChild(root, cone_group);
    
    // Create cones in a circle
    for (int i = 0; i < 5; ++i) {
        // Transform node for positioning
        uint32_t cone_transform = createTransformNode(graph, "cone" + std::to_string(i+1) + "_transform");
        float angle = (i * 72.0f) * M_PI / 180.0f;
        glm::vec3 position(2.5f * cos(angle), 0, 2.5f * sin(angle));
        graph.setTranslation(cone_transform, position);
        graph.addChild(cone_group, cone_transform);
        
        // Cone geometry
        uint32_t cone = createGeometryNode(graph, "cone" + std::to_string(i+1), GeometryType::Cone);
        glm::vec3 color(0.8f, 0.2f + i * 0.15f, 0.2f + i * 0.1f);
        graph.addGeometryComponent(cone, createConeGeometry(0.5f, 1.5f, 16, color));
        graph.addChild(cone_transform, cone);
    }
    
    // Create primitives group with color saving
    uint32_t primitives_group = createSeparatorNode(graph, "primitives_group", true, false, true);
    graph.addChild(root, primitives_group);
    
    // Sphere with transform
    uint32_t sphere_transform = createTransformNode(graph, "sphere_transform");
    graph.setTranslation(sphere_transform, glm::vec3(1.5f, 1.5f, 1.5f));
    graph.setRotation(sphere_transform, glm::vec3(0, 45, 0));
    graph.addChild(primitives_group, sphere_transform);
    
    uint32_t sphere = createGeometryNode(graph, "sphere", GeometryType::Sphere);
    graph.addGeometryComponent(sphere, createSphereGeometry(0.7f, 32, 16, glm::vec3(0.8f, 0.2f, 0.8f)));
    graph.addChild(sphere_transform, sphere);
    
    // Cylinder with transform
    uint32_t cylinder_transform = createTransformNode(graph, "cylinder_transform");
    graph.setTranslation(cylinder_transform, glm::vec3(-1.5f, 0, 1.5f));
    graph.setRotation(cylinder_transform, glm::vec3(15, 0, 0));
    graph.addChild(primitives_group, cylinder_transform);
    
    uint32_t cylinder = createGeometryNode(graph, "cylinder", GeometryType::Cylinder);
    graph.addGeometryComponent(cylinder, createCylinderGeometry(0.5f, 1.5f, glm::vec3(0.2f, 0.8f, 0.8f)));
    graph.addChild(cylinder_transform, cylinder);
    
    // Cube with transform
    uint32_t cube_transform = createTransformNode(graph, "cube_transform");
    graph.setTranslation(cube_transform, glm::vec3(1.5f, 0.5f, -1.5f));
    graph.setRotation(cube_transform, glm::vec3(0, 30, 0));
    graph.setScale(cube_transform, 1.2f);
    graph.addChild(primitives_group, cube_transform);
    
    uint32_t cube = createGeometryNode(graph, "cube", GeometryType::Cube);
    graph.addGeometryComponent(cube, createCubeGeometry(1.0f, glm::vec3(0.9f, 0.5f, 0.1f)));
    graph.addChild(cube_transform, cube);
    
    // Complex hierarchical transforms
    uint32_t complex_group = createSeparatorNode(graph, "complex_group", true, false, false);
    graph.addChild(root, complex_group);
    
    uint32_t parent_transform = createTransformNode(graph, "parent_transform");
    graph.setTranslation(parent_transform, glm::vec3(-2.0f, 1.0f, -2.0f));
    graph.addChild(complex_group, parent_transform);
    
    uint32_t child_transform = createTransformNode(graph, "child_transform");
    graph.setTranslation(child_transform, glm::vec3(0, 1.0f, 0));
    graph.setScale(child_transform, 0.9f);
    graph.addChild(parent_transform, child_transform);
    
    uint32_t sphere2 = createGeometryNode(graph, "sphere2", GeometryType::Sphere);
    graph.addGeometryComponent(sphere2, createSphereGeometry(0.9f, 32, 16, glm::vec3(0.1f, 0.9f, 0.4f)));
    graph.addChild(child_transform, sphere2);
    
    // Nested group with complex transforms
    uint32_t nested_group = createSeparatorNode(graph, "nested_group", true, false, false);
    graph.addChild(root, nested_group);
    
    uint32_t outer_transform = createTransformNode(graph, "outer_transform");
    graph.setTranslation(outer_transform, glm::vec3(-3.0f, 0.6f, 0.5f));
    graph.setRotation(outer_transform, glm::vec3(0, 45, 0));
    graph.addChild(nested_group, outer_transform);
    
    uint32_t inner_transform = createTransformNode(graph, "inner_transform");
    graph.setRotation(inner_transform, glm::vec3(45, 0, 45));
    graph.setScale(inner_transform, 0.8f);
    graph.addChild(outer_transform, inner_transform);
    
    uint32_t cube2 = createGeometryNode(graph, "cube2", GeometryType::Cube);
    graph.addGeometryComponent(cube2, createCubeGeometry(1.0f, glm::vec3(0.7f, 0.1f, 0.7f)));
    graph.addChild(inner_transform, cube2);
}

// Performance comparison example
void performanceTest() {
    std::cout << "=== Performance Comparison ===\n";
    
    ModernSceneGraph modern_graph;
    createModernScene(modern_graph);
    
    // Simulate many draw calls
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        modern_graph.updateTransforms();
        // modern_graph.draw(); // Would draw in real scenario
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Modern approach: " << duration.count() << " microseconds for 1000 updates\n";
    
    std::cout << "\nModern Scene Graph Benefits:\n";
    std::cout << "- No virtual function calls\n";
    std::cout << "- Better cache locality (SoA layout)\n";
    std::cout << "- Efficient component-based design\n";
    std::cout << "- Flat memory layout for transforms\n";
    std::cout << "- Easy to parallelize updates\n";
    std::cout << "- Lower memory overhead per node\n";
    std::cout << "===============================\n\n";
}

int main() {
    ModernSceneGraph graph;
    createModernScene(graph);
    
    // Print the hierarchy
    graph.printHierarchy();
    
    // Run performance test
    performanceTest();
    
    return 0;
}
