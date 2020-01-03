# OpenSceneGraph test

## Basic demo

- Define a model in a script
    - Add ground plane and a few objects for reference
    - Add wemo body and wheels at kinematically correct locations
- Animate the model
    - Get reference to nodes in the model defined in script
    - Use wemo kinematics to move robot at a desired speed
    - Rotate wheels programmatically as per kinematic model
- User interaction
    - rotate
    - translate
    - click on robot to show overlay with speed info
- Show viewer as a Qt widget
- Build application using cmake

## Advanced Demo

- Define a front facing camera on robot
- Show scene view and robot camera view simultaneously

## Notes

- To install SDL2 without causing dependency conflicts with pulseaudio
```
wget http://archive.ubuntu.com/ubuntu/ubuntu/pool/main/p/pulseaudio/libpulse-dev_11.1-1ubuntu7.5_amd64.deb
sudo dpkg -i ./libpulse-dev_11.1-1ubuntu7.5_amd64.deb
sudo apt install libsdl2-dev
```

- Use `osg::NotifyHandler` to redirect trace messages into custom handler
