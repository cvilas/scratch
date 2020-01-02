#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/Tessellator>
#include <osgViewer/Viewer>
#include <iostream>

/// Complex primitives will not be rendered correctly by the OpenGL API directly. This include
/// concave polygons, self-intersecting polygons, and polygons with holes. Only after being
/// subdivided into convex polygons, these non-convex polygons can be accepted by the OpenGL
/// rendering pipeline. The osgUtil::Tessellator class can be used for the tessellation work in this case.
int main(int argc, char* argv[])
{
    osg::ArgumentParser args(&argc, argv);
    args.getApplicationUsage()->addCommandLineOption("-t","Tessellate");
    unsigned int helpType = 0;
    if ((helpType = args.readHelpType()))
    {
        args.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    /// We will create a concave polygon by using the osg::Geometry class. A simple
    /// polygon is concave if any of its internal edge angles is greater than 180 degrees.
    /// Here, the example geometry represents a quad with a cave on the right-hand side.
    /// It is drawn as a GL_POLYGON primitive.
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 0.0f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 1.0f) );
    vertices->push_back( osg::Vec3(1.0f, 0.0f, 2.0f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 2.0f) );
    vertices->push_back( osg::Vec3(2.0f, 0.0f, 3.0f) );
    vertices->push_back( osg::Vec3(0.0f, 0.0f, 3.0f) );

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0.0f,-1.0f, 0.0f) );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->setNormalArray( normals.get() );
    geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
    geom->addPrimitiveSet( new osg::DrawArrays(GL_POLYGON, 0, 8) );

    // To render the concave polygon correctly, we should re-tessellate it:
    if(args.read("-t"))
    {
        osgUtil::Tessellator tessellator;
        tessellator.retessellatePolygons( *geom );
        OSG_NOTICE << args.getApplicationName() << ": Tessellating concave polygon\n";
    }
    else
    {
        OSG_NOTICE << args.getApplicationName() << ": NOT tessellating concave polygon. Rendering incorrect\n";
    }

    // Add it to a geometry node and start the scene viewer:
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( geom.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
