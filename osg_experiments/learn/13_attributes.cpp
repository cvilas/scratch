/// We will show the usage of the OVERRIDE and PROTECTED flags in the following short
/// example. The root node will be set to OVERRIDE, in order to force all children to inherit
/// its attribute or mode. Meanwhile, the children will try to change their inheritance with or
/// without a PROTECTED flag, which will lead to different results.

#include <osg/PolygonMode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main()
{
    /// Create two osg::MatrixTransform nodes and make them both share a glider
    /// model. After all, we don't want to use the well-known Cessna all the time. The glider
    /// is small in size, so only a small distance is required for the setMatrix() method:
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("glider.osg" );
    osg::ref_ptr<osg::MatrixTransform> transformation1 = new osg::MatrixTransform;
    transformation1->setMatrix(osg::Matrix::translate(-0.5f, 0.0f, 0.0f));
    transformation1->addChild( model.get() );
    osg::ref_ptr<osg::MatrixTransform> transformation2 = new osg::MatrixTransform;
    transformation2->setMatrix(osg::Matrix::translate(0.5f, 0.0f, 0.0f));
    transformation2->addChild( model.get() );

    /// Add the two transformation nodes to the root:
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( transformation1.get() );
    root->addChild( transformation2.get() );

    /// Now we are going to set the rendering mode of each node's state set. The
    /// GL_LIGHTING mode is a famous OpenGL enumeration which can be used to
    /// enable or disable global lighting of the scene. Note that the OVERRIDE and
    /// PROTECTED flags are set to root and transformation2 separately, along
    /// with an ON or OFF switch value:
    transformation1->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    transformation2->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED);
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE );

    /// Start the viewer:
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();

    /// In this example, we are trying to change the GL_LIGHTING modes of transformation1
    /// and transformation2 to disable lights on them. Meanwhile, we have turned on the
    /// lighting mode for the root node, and used an OVERRIDE flag for all children to follow to
    /// retain their lighting states.
    /// The node transformation1, as shown in the previous image, remains lighted in spite of
    /// its own setting. However, transformation2 uses a PROTECTED flag to prevent itself from
    /// being affected by the root. It becomes a little brighter as a result of "turning off" the light on
    /// its surfaces. This is simply because the geometries are now directly colored according to the
    /// original color arrays, without any more reaction to the lights.

}
