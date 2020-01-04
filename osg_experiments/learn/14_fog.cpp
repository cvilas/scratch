/// We will take the fog effect as an ideal example of working with various rendering attributes
/// and modes. OpenGL accepts one linear and two exponential fog equations, which are
/// supported by the osg::Fog class as well.
#include <osg/Fog>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main()
{
    /// We would like to first create the fog attribute. Using the linear mode, we have to
    /// set the near and far distances by using setStart() and setEnd() methods.
    /// We will also set the fog color, in order to generate a dust fog-like effect:
    osg::ref_ptr<osg::Fog> fog = new osg::Fog;
    fog->setMode( osg::Fog::LINEAR );
    fog->setStart( 500.0f );
    fog->setEnd( 2500.0f );
    fog->setColor( osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f) );

    /// We are going to load an example terrain model named lz.osg, which can
    /// be located in the data directory indicated by the environment variable
    /// OSG_FILE_PATH. The only work to do is to set the fog attribute and the
    /// associated mode to the node's state set.
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( "lz.osg" );
    model->getOrCreateStateSet()->setAttributeAndModes( fog.get() );

    /// Start the viewer and manipulate the scene, in order to make the terrain and
    /// the fog have a better appearance:
    osgViewer::Viewer viewer;
    viewer.setSceneData( model.get() );
    return viewer.run();

    /// As you scale the scene by using the right mouse button, the terrain model
    /// will fade in and out of the fog in a smooth progression. This is a very basic
    /// environment effect, but the result can still be fantastic sometimes.
}
