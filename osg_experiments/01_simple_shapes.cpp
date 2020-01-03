#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osgViewer/Viewer>

/// The osg::Geode class corresponds to the leaf node of a scene graph. It has no child
/// nodes, but always contains geometry information for rendering. Its name Geode is short
/// for geometry node.
/// The geometry data to be drawn are stored in a set of osg::Drawable objects managed
/// by osg::Geode. The non-instantiatable osg::Drawable class is defined as a pure virtual
/// class. It has several subclasses for rendering models, images, and texts to the OpenGL
/// pipeline. These renderable elements are collectively called drawables.
///
/// The osg::ShapeDrawable class is useful for quick display, but it is not an efficient
/// way of drawing geometry primitives. It should only be used for quick prototyping and
/// debugging when you develop 3D applications. To create geometries with high performance
/// computation and visualization requirements, the osg::Geometry class, which is going to be
/// introduced, is always a better choice.

int main()
{
    osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable;
    shape1->setShape( new osg::Box(osg::Vec3(-3.0f, 0.0f, 0.0f), 2.0f, 2.0f, 1.0f) );

    osg::ref_ptr<osg::ShapeDrawable> shape2 = new osg::ShapeDrawable;
    shape2->setShape( new osg::Sphere(osg::Vec3(3.0f, 0.0f, 0.0f), 1.0f) );
    shape2->setColor( osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) );

    osg::ref_ptr<osg::ShapeDrawable> shape3 = new osg::ShapeDrawable;
    shape3->setShape( new osg::Cone(osg::Vec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f) );
    shape3->setColor( osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) );

    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( shape1.get());
    root->addDrawable( shape2.get());
    root->addDrawable( shape3.get());

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
