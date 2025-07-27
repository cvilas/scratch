# SceneGraph Demo with Advanced Architecture Implementations

A comprehensive 3D scene graph demonstration showcasing both traditional OOP hierarchy and modern data-oriented design approaches, with interactive mouse-controlled camera movement and advanced lighting systems.

## Project Overview

This project demonstrates the evolution from traditional object-oriented scene graph design to modern, performance-optimized architectures used in contemporary game engines. It includes three different implementations:

1. **Traditional OOP Scene Graph** (`scenegraph_demo`) - Classic inheritance-based hierarchy
2. **Modern Scene Graph Console Demo** (`modern_demo`) - Performance-optimized flat structure  
3. **Modern Scene Graph SDL3 Renderer** (`modern_sdl_demo`) - Complete interactive application with modern architecture

## Architecture Implementations

### Traditional OOP Approach
- **Inheritance-based hierarchy** with virtual function dispatch
- **Array of Structures (AoS)** memory layout
- **shared_ptr references** for node relationships
- **Polymorphic rendering** through virtual draw() methods

### Modern Data-Oriented Approach  
- **Flat Structure of Arrays (SoA)** for cache efficiency
- **Component-based design** with sparse component storage
- **Index-based relationships** instead of pointer indirections
- **Direct function dispatch** eliminating virtual call overhead
- **~2-3x performance improvement** for large scenes

## Features

### Core Scene Graph Features
- **Hierarchical Scene Organization**: Transform and Separator nodes for professional scene management
- **Advanced Node Types**: Transform nodes for decoupled transformations, Separator nodes for OpenGL state isolation
- **Component System**: Modern component-based architecture with geometry, lighting, and camera components
- **Multiple Primitive Types**: Cubes, spheres, cylinders, cones, and planes with customizable properties

### Interactive Controls
- **Mouse Camera Controls**: Interactive camera movement around the scene with spherical coordinates
- **Mouse Capture System**: Toggle between camera control and normal mouse usage
- **Zoom Controls**: Mouse scroll wheel for distance adjustment
- **Window Management**: Resizable windows with automatic aspect ratio adjustment

### Advanced Lighting System
- **Dual Dynamic Lighting**: Camera-attached spotlight + animated moving spotlight
- **Realistic Material Properties**: Proper diffuse, ambient, specular, and shininess
- **Real-time Animation**: Sinusoidal movement patterns with time-based updates
- **Global Ambient Lighting**: Prevents completely dark areas

### Technical Implementations
- **SDL3 Integration**: Modern windowing and input handling
- **OpenGL Fixed Pipeline**: Immediate mode rendering with proper lighting
- **GLM Mathematics**: Professional-grade matrix and vector operations
- **Performance Optimization**: Modern data-oriented design principles

## Controls

- **Mouse Movement**: Rotate the camera around the scene center (when mouse is captured)
- **Mouse Scroll**: Zoom in/out (adjust camera distance, when mouse is captured)
- **TAB Key**: Toggle mouse capture on/off (allows you to use mouse for other windows)
- **ESC Key**: Exit the application
- **Window Resize**: Drag window edges to resize - the scene will automatically adjust

## Building

### Prerequisites
- **C++23 compatible compiler** (GCC 12+, Clang 15+)
- **CMake 3.10+**
- **SDL3 development libraries**
- **OpenGL development libraries** (GL, GLU)
- **GLM mathematics library**

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

## Running the Demos

### Traditional OOP Scene Graph (GLFW)
```bash
./scenegraph_demo
```
Interactive SDL3 application with traditional inheritance-based scene graph.

### Modern Console Demo  
```bash
./modern_demo
```  
Console application demonstrating modern architecture with performance benchmarks.

### Modern SDL3 Scene Graph
```bash
./modern_sdl_demo
```
Interactive SDL3 application with modern data-oriented scene graph architecture.

## Performance Comparison

| Metric | Traditional OOP | Modern Approach | Improvement |
|--------|----------------|-----------------|-------------|
| **Memory per Node** | ~120+ bytes | ~80 bytes | ~33% reduction |
| **Transform Updates** | ~40,000μs/1000 | ~18,000μs/1000 | ~2.2x faster |
| **Cache Misses** | High (scattered) | Low (contiguous) | Significant |
| **Virtual Calls** | Every draw() | None | 100% eliminated |

## Camera System

The camera uses a spherical coordinate system centered around the origin (0,0,0):
- **Yaw**: Horizontal rotation around the Y-axis
- **Pitch**: Vertical rotation (limited to ±89 degrees to prevent camera flipping)
- **Distance**: Zoom level from the target point

### Mouse Capture System

The application uses a mouse capture system that allows you to toggle between:
- **Captured Mode**: Mouse is locked to the window and controls the camera
- **Normal Mode**: Mouse cursor is visible and can be used for other windows

Press **TAB** to toggle between these modes. This is especially useful when working with multiple applications.

### Window Management

The application supports dynamic window resizing:
- **Viewport**: Automatically adjusts to fill the entire window
- **Aspect Ratio**: Projection matrix updates to maintain proper proportions
- **Mouse Coordinates**: Mouse capture center adjusts to new window dimensions

### Lighting System

The application features a sophisticated dual-lighting system:

- **Camera Spotlight (GL_LIGHT0)**: 
  - Warm white illumination (1.0, 0.95, 0.8)
  - Follows camera position and direction
  - 30° cutoff angle with smooth falloff
  
- **Moving Overhead Spotlight (GL_LIGHT1)**:
  - Cool blue-white light (0.9, 0.9, 1.0) 
  - Sinusoidal movement pattern from above
  - 25° cutoff angle with focused beam
  - Creates dynamic shadows and highlights

- **Material Properties**: Each object has realistic diffuse, ambient, specular, and shininess values
- **Global Ambient**: 25% base illumination prevents completely dark areas
- **Two-Sided Lighting**: Objects illuminated from both sides for better visibility

## Development Evolution

This project showcases the evolution of graphics programming approaches:

### 1. **Initial Implementation** (Traditional OOP)
- Started with classic inheritance-based scene graph
- GLFW for windowing and input handling
- Virtual function dispatch for polymorphic rendering

### 2. **SDL Migration** (API Modernization)  
- Migrated from GLFW to SDL2, then SDL3
- Updated event handling paradigms
- Addressed breaking API changes in SDL3

### 3. **Modern Architecture** (Performance Optimization)
- Implemented data-oriented design principles
- Structure of Arrays (SoA) memory layout
- Component-based entity system
- Eliminated virtual function overhead

## Educational Value

This project serves as a comprehensive example of:

- **Scene Graph Design Patterns**: Traditional vs modern approaches
- **Performance Optimization**: Cache-friendly data structures and algorithms
- **API Migration**: Handling library upgrades and breaking changes  
- **Modern C++**: C++23 features, smart pointers, RAII principles
- **Graphics Programming**: OpenGL lighting, transformations, and rendering
- **Software Architecture**: Evolution from OOP to data-oriented design

## When to Use Each Approach

### Traditional OOP Scene Graph
- ✅ **Small scenes** (<100 nodes)
- ✅ **Rapid prototyping** and learning
- ✅ **Diverse node behaviors** requiring polymorphism
- ✅ **Educational purposes** and demonstrations

### Modern Data-Oriented Scene Graph  
- ✅ **Large scenes** (>1000 nodes)
- ✅ **Performance-critical applications**
- ✅ **Game engines** and real-time graphics
- ✅ **Memory-constrained environments**
- ✅ **Parallelization requirements**

## Future Enhancements

The modern architecture enables several advanced optimizations:

- **SIMD Batch Processing**: Update multiple transforms with AVX/SSE
- **GPU Upload Optimization**: Direct memcpy of transform arrays
- **Spatial Indexing**: Integration with octrees/BVH for culling
- **Job System Integration**: Parallelize updates across CPU cores  
- **Memory Pooling**: Custom allocators for optimal memory usage

## Technical Architecture

### Traditional OOP Implementation
```cpp
// Inheritance-based hierarchy
class Node {
    virtual void draw(const glm::mat4& transform) = 0;  // Virtual dispatch
    std::vector<std::shared_ptr<Node>> children;        // Pointer indirection
};

class Cube : public Node {
    void draw(const glm::mat4& transform) override;     // Polymorphic behavior
};
```

### Modern Data-Oriented Implementation  
```cpp
// Flat structure with components
class ModernSceneGraph {
    std::vector<glm::mat4> local_transforms_;      // Structure of Arrays (SoA)
    std::vector<NodeType> node_types_;             // Cache-friendly layout
    std::unordered_map<uint32_t, GeometryComponent> geometry_components_;  // Sparse storage
    
    void draw() {
        // Direct dispatch, no virtual calls
        for (uint32_t node : geometry_nodes_) {
            drawGeometry(node);  // Type-specific function
        }
    }
};
```

### Memory Layout Comparison

**Traditional (Array of Structures):**
```
Node1: [vtable*][transform][children][...] → Heap allocation
Node2: [vtable*][transform][children][...] → Heap allocation  
Node3: [vtable*][transform][children][...] → Heap allocation
```

**Modern (Structure of Arrays):**
```  
Transforms: [mat4][mat4][mat4][mat4]... → Contiguous memory
Types:      [u8][u8][u8][u8]...        → Contiguous memory
Components: {sparse hash map}          → Only when needed
```

## SDL Migration Journey

The project demonstrates the migration from GLFW to SDL3, showcasing:

### GLFW → SDL2 → SDL3 Evolution
- **Event System**: GLFW callbacks → SDL event polling → SDL3 modern events
- **Window Management**: Different APIs for creation and context management  
- **Input Handling**: Callback-based → polling-based → modern SDL3 events
- **API Modernization**: Boolean constants, function renames, simplified signatures

### Key SDL3 Changes Addressed
- `SDL_TRUE`/`SDL_FALSE` → `true`/`false`
- `SDL_SetRelativeMouseMode()` → `SDL_SetWindowRelativeMouseMode()`
- Event constants: `SDL_QUIT` → `SDL_EVENT_QUIT`
- Simplified `SDL_CreateWindow()` signature
- `SDL_GL_DeleteContext()` → `SDL_GL_DestroyContext()`
