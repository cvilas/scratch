#include <osg/Geometry>
#include <osg/Geode>
#include <osgUtil/SmoothingVisitor>
#include <osgViewer/Viewer>

int main()
{
    /// Define the vertices of the octahedron
    ///             3
    ///            / \
    ///           /   \
    ///    3 --- 4 --- 0 --- 3
    ///   / \   / \   / \   /
    ///  /   \ /   \ /   \ /
    /// 2 --- 5 --- 1 --- 2
    ///        \   /
    ///         \ /
    ///          2
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(6);
    (*vertices)[0].set( 0.0f, 0.0f, 1.0f);
    (*vertices)[1].set(-0.5f,-0.5f, 0.0f);
    (*vertices)[2].set( 0.5f,-0.5f, 0.0f);
    (*vertices)[3].set( 0.5f, 0.5f, 0.0f);
    (*vertices)[4].set(-0.5f, 0.5f, 0.0f);
    (*vertices)[5].set( 0.0f, 0.0f,-1.0f);

    /// Specify the indices of vertices to describe all eight triangle faces.
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES, 24);
    (*indices)[0] = 0; (*indices)[1] = 1; (*indices)[2] = 2;
    (*indices)[3] = 0; (*indices)[4] = 2; (*indices)[5] = 3;
    (*indices)[6] = 0; (*indices)[7] = 3; (*indices)[8] = 4;
    (*indices)[9] = 0; (*indices)[10]= 4; (*indices)[11]= 1;
    (*indices)[12]= 5; (*indices)[13]= 2; (*indices)[14]= 1;
    (*indices)[15]= 5; (*indices)[16]= 3; (*indices)[17]= 2;
    (*indices)[18]= 5; (*indices)[19]= 4; (*indices)[20]= 3;
    (*indices)[21]= 5; (*indices)[22]= 1; (*indices)[23]= 4;

    /// In order to create a geometry with a default white color, we will only set the vertex array
    /// and the osg::DrawElementsUInt primitive set. The normal array is also required but is not easy to
    /// compute manually. We will use a smoothed normal calculator to automatically obtain it.
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->addPrimitiveSet( indices.get() );
    osgUtil::SmoothingVisitor::smooth( *geom );

    /// Add the geometry to an osg::Geode object and make it the scene root:
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( geom.get() );
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
