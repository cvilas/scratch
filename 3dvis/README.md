# Visualisation test

## Tested frameworks
- Qt3D
- OpenSceneGraph

## Basic demo

- Define kinematically correct model
- Define DDS odometry receiver node
- Animate the model
    - body movement with odometry pose
    - wheels rotation with odometry speeds and kinematics
- Build application using cmake

## Advanced Demo

- Define model in a script
- Animate model by applying transforms to nodes defined in script
- Add environment objects
- Define a front facing camera on robot
- Show scene view and robot camera view simultaneously
- User interaction
    - rotate
    - translate
    - click on robot to show overlay with speed info
- Show viewer as a Qt widget

## Notes

- To install SDL2 without causing dependency conflicts with pulseaudio
```
wget http://archive.ubuntu.com/ubuntu/ubuntu/pool/main/p/pulseaudio/libpulse-dev_11.1-1ubuntu7.5_amd64.deb
sudo dpkg -i ./libpulse-dev_11.1-1ubuntu7.5_amd64.deb
sudo apt install libsdl2-dev
```
- Use gltf converter: https://blackthread.io/gltf-converter/
- Use `osg::NotifyHandler` to redirect trace messages into custom handler
