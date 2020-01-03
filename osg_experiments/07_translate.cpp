#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>

int main()
{
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("cessna.osg" );

    /// All child nodes will be affected by the MatrixTransform node and be transformed according to the
    /// presetting matrix. Here, we will transform the loaded model twice, in order to obtain two instances
    /// displayed separately at the same time:
    ///
    /// Resulting scenegraph:
    ///
    ///            root
    ///             |
    ///    +--------+-------+
    ///    |                |
    /// transform1      transform2
    ///    |                |
    ///    +--------+-------+
    ///             |
    ///      cessna (shared)
    ///
    /// The model (cessna) is rendered twice under context of parent nodes, as if two different objects.
    /// Sharing leaf nodes, geometries, textures, and OpenGL rendering states as much as possible
    /// improves rendering effciency

    /// PositionAttitudeTransform is easier to use than MatrixTransform
    osg::ref_ptr<osg::PositionAttitudeTransform> transform1 = new osg::PositionAttitudeTransform;
    transform1->setPosition( osg::Vec3d(-25.0f, 0.0f, 0.0f) );
    transform1->addChild( model.get() );

    osg::ref_ptr<osg::MatrixTransform> transform2 = new osg::MatrixTransform;
    transform2->setMatrix( osg::Matrix::translate(25.0f, 0.0f, 0.0f) );
    transform2->addChild( model.get() );

    /// Add the two transformation nodes to the root node and start the viewer:
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( transform1.get() );
    root->addChild( transform2.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();

}
