/// We are going to select the polygon rasterization mode of a loaded model. The
/// osg::PolygonMode class, which is derived from the osg::StateAttribute base class,
/// will be used to achieve this goal. It simply encapsulates OpenGL's glPolygonMode()
/// function and implements interfaces for specifying face and drawing mode parameters, and
/// thus changes the final rasterization of the attached node.
#include <osg/PolygonMode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main()
{
    /// Create two osg::MatrixTransform nodes and make them share the same
    /// loaded Cessna model. The two transformation nodes are placed at different
    /// positions in the 3D world, which will display two Cessna models as the result:
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("cessna.osg" );
    osg::ref_ptr<osg::MatrixTransform> transformation1 = new osg::MatrixTransform;
    transformation1->setMatrix(osg::Matrix::translate(-25.0f,0.0f,0.0f));
    transformation1->addChild( model.get() );

    osg::ref_ptr<osg::MatrixTransform> transformation2 = new osg::MatrixTransform;
    transformation2->setMatrix(osg::Matrix::translate(25.0f,0.0f,0.0f));
    transformation2->addChild( model.get() );

    /// Now, we will add an osg::PolygonMode rendering attribute to the associated
    /// state set of the node transformation1. It has a setMode() method which
    /// accepts two parameters: the face that the mode applied to, and the mode should
    /// be in which polygons will be rasterized:
    osg::ref_ptr<osg::PolygonMode> pm = new osg::PolygonMode;
    pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
    transformation1->getOrCreateStateSet()->setAttribute( pm.get() );

    /// The next step is familiar. Now we can add the nodes to a root node, and start the
    /// viewer to see if anything has changed:
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( transformation1.get() );
    root->addChild( transformation2.get() );
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();

    /// The Cessna model at the position (-25.09, 0.0, 0.0), or on the left of the
    /// initial display window, is drawn with outlined front and back facing polygons.
    /// On the contrary, the model on the right is still fully filled as usual:
}
