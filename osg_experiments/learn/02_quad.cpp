#include <osg/Geometry>
#include <osg/Geode>
#include <osgViewer/Viewer>

/// Demonstrates creating a drawable from a raw set of vertices with associated normals and colours
int main()
{
    /// Create the vertex array and push the four corner points to the back of the array
    /// by using std::vector like operations:
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 1.0f) );

    /// define the normal for each vertex; otherwise OpenGL will use a default (0, 0, 1) normal vector
    /// and the lighting equation calculation may be incorrect. The four vertices actually face the
    /// same direction, so a single normal vector is enough. We will also set the setNormalBinding()
    /// method to BIND_OVERALL later.
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0.0f,-1.0f, 0.0f) );

    /// Define color value to each vertex. By default, OpenGL will use smooth coloring and blend
    /// colors at each vertex together
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back( osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) );
    colors->push_back( osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) );
    colors->push_back( osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) );
    colors->push_back( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );

    /// Create geometry object and set the prepared vertex, normal, and color arrays to it.
    /// Also bind single normal to the entire geometry and bind colors per vertex:
    osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
    quad->setVertexArray( vertices.get() );
    quad->setNormalArray( normals.get() );
    quad->setNormalBinding( osg::Geometry::BIND_OVERALL );
    quad->setColorArray( colors.get() );
    quad->setColorBinding( osg::Geometry::BIND_PER_VERTEX );

    /// To finish a geometry and add it to the scene graph, specify the primitive set.
    /// Drawing mode GL_QUADS renders the four vertices as quad corners in a counter-clockwise order:
    quad->addPrimitiveSet( new osg::DrawArrays(GL_QUADS, 0, 4) );

    /// Render the geometry in the scene viewer:
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( quad.get() );
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
