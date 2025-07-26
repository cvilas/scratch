# SceneGraph Demo with Mouse Camera Controls

A simple 3D scene graph demonstration with interactive mouse-controlled camera movement.

## Features

- **Scene Graph**: Hierarchical node structure for organizing 3D objects
- **Mouse Camera Controls**: Interactive camera movement around the scene
- **Dynamic Spotlight**: Camera-attached spotlight that illuminates the scene
- **Moving Spotlight**: Animated overhead spotlight that moves in sinusoidal patterns
- **Realistic Lighting**: Proper material properties, specular highlights, and ambient lighting
- **Multiple Objects**: Several colored cones and a ground plane to demonstrate lighting
- **Resizable Window**: Window can be resized and scene adjusts automatically
- **Mouse Capture Toggle**: Switch between camera control and normal mouse usage

## Controls

- **Mouse Movement**: Rotate the camera around the scene center (when mouse is captured)
- **Mouse Scroll**: Zoom in/out (adjust camera distance, when mouse is captured)
- **TAB Key**: Toggle mouse capture on/off (allows you to use mouse for other windows)
- **ESC Key**: Exit the application
- **Window Resize**: Drag window edges to resize - the scene will automatically adjust

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./scenegraph_demo
```

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

The application features a sophisticated lighting system:
- **Camera Spotlight**: A spotlight attached to the camera position that follows your view
- **Moving Overhead Spotlight**: An animated blue-white spotlight that moves in sinusoidal patterns from above
- **Global Ambient Light**: Provides base illumination (25% intensity) to prevent areas from being completely dark
- **Two-Sided Lighting**: Objects are illuminated from both sides for better visibility
- **Material Properties**: Objects have realistic diffuse, ambient, specular, and shininess properties
- **Ground Plane**: Checkerboard ground plane with enhanced reflectivity to show lighting effects
- **Dual Light Colors**: Warm white camera light (1.0, 0.95, 0.8) and cool blue-white moving light (0.8, 0.9, 1.0)
- **Dynamic Animation**: Moving light creates shifting shadows and highlights in real-time

## Technical Details

- Built with C++23
- Uses OpenGL for rendering with fixed-function pipeline lighting
- GLFW for window management and input handling
- GLM for mathematical operations
- **Dual OpenGL Lighting**: GL_LIGHT0 (camera spotlight) + GL_LIGHT1 (moving spotlight)
- **Material System**: Proper diffuse, specular, and shininess material properties
- **Normal Vectors**: Calculated per-triangle normals for realistic lighting
- **Real-time Animation**: Sinusoidal movement patterns with time-based updates
- Immediate mode OpenGL rendering (for simplicity)
