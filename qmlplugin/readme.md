# Custom QML plugin development example

To create a basic plugin:

- Need to subclass QQuickItem or QQuickPaintedItem (if custom draw is required)
- Need a `qmldir` file with the name of the module `module_name` and the name of the plugin (`plugin`, if the library
  file generated is `libplugin.so`)
- The library must be installed in a <location> with directory strucuture `location/module_name` which should
  contain `qmldir` file and the plugin library `libplugin.so`
- The qml file contains a line to import the module. eg: `import module_name 1.0`
- Link the executable against the plugin library. For the qmlapplication engine to find the module, set
  engine.addImportPath to `location`. The engine will look for `module_name` in the subdirectories

To show a list of objects thus created:

- TODO use a ListView and a data model to attach it to

To communicate with individual items in the list and update their views:

- TODO


## Notes to QML and C++ Integration

* The C++ object to expose must be derived from QObject

* An instance of the class can be embedded into QML code as a context property or context object, allowing the
  instance's properties, methods and signals to be accessed from QML. See
  https://doc.qt.io/qt-5.10/qtqml-cppintegration-contextproperties.html

* or, register the class as an instantiable QML type so that it can be created like any other QML object type:
  https://doc.qt.io/qt-5.10/qtqml-cppintegration-definetypes.html#registering-an-instantiable-object-type

* To manipulate QML objects with C++ code, see: https://doc.qt.io/qt-5.10/qtqml-cppintegration-interactqmlfromcpp.html

* To integrate C++ object into a plugin, see: https://doc.qt.io/qt-5.10/qtqml-modules-cppplugins.html

* Warning: Although it is possible to access QML objects from C++ and manipulate them (using QObject::findChildren),
it is not the recommended approach, except for testing and prototyping purposes. One of the strengths of QML and C++
integration is the ability to implement UIs in QML separate from the C++ logic and dataset backend, and this fails if
the C++ side starts manipulating QML directly. Such an approach also makes changing the QML UI difficult without
affecting its C++ counterpart.

* What the above means is make the GUI components stand-alone. Don't embed your C++ object in them.
