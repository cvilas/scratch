#include <osg/Geometry>
#include <osg/Geode>
#include <osgViewer/Viewer>
#include <osg/TriangleFunctor>
#include <iostream>

/// The osg::TriangleFunctor<> functor class is ideal for collecting information on triangle
/// faces. It will convert primitive sets of an osg::Drawable object to triangles whenever
/// possible. The template argument must implement an operator(), which will be called for
/// every triangle when the functor is applied.

std::ostream& operator<<(std::ostream& os, const osg::Vec3& v)
{
    os << v[0] << ", " << v[1] << ", " << v[2];
    return os;
}

/// The functor
struct FaceCollector
{
void operator()( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3 )
{
    std::cout << "Face vertices: " << v1 << "; " << v2 << "; " << v3 << std::endl;
}
};

int main()
{
    /// We will create a wall-like object by using GL_QUAD_STRIP, which means that the
    /// geometry was not originally formed by triangles. This object includes eight vertices
    /// and four quad faces:
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 1.5f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(3.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(3.0f, 0.0f, 1.5f) );
    vertices->push_back( osg::Vec3(4.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(4.0f, 0.0f, 1.0f) );

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0.0f,-1.0f, 0.0f) );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_QUAD_STRIP, 0, 10) );

    /// Now, add the user-defined FaceCollector and apply it to the osg::Geometry object:
    /// Console will see a list of face vertices
    osg::TriangleFunctor<FaceCollector> functor;
    geom->accept( functor );

    /// view the object. It is nothing special when compared to previous geometries:
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( geom.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
