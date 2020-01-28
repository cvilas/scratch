/// By default, OSG automatically turns on the first light (GL_LIGHT0) and gives the scene a soft,
/// directional light. However, this time we will create multiple lights by ourselves, and move
/// them with transformation parent nodes. Be aware: only positional lights can be translated.
/// A directional light has no origin and cannot be placed anywhere.
/// OpenGL and OSG both use the fourth component of the position parameter to decide if
/// a light is a point light. That is to say, if the fourth component is 0, the light is treated as a
/// directional source; otherwise it is positional.

#include <osg/MatrixTransform>
#include <osg/LightSource>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

/// We create a function to create light sources for the scene graph. A light source
/// should have a number (ranging from 0 to 7), a translation position, and a color
/// parameter. A point light is created because the fourth part of the position vector
/// is 1.0. After that, we assign the light to a newly-created osg::LightSource
/// node, and add the light source to a translated osg::MatrixTransform node,
/// which is then returned:
osg::Node* createLightSource( unsigned int num, const osg::Vec3& trans, const osg::Vec4& color )
{
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum( num );
    light->setDiffuse( color );
    light->setPosition( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight( light );
    osg::ref_ptr<osg::MatrixTransform> sourceTrans = new osg::MatrixTransform;
    sourceTrans->setMatrix( osg::Matrix::translate(trans) );
    sourceTrans->addChild( lightSource.get() );
    return sourceTrans.release();
}

int main()
{
    /// The Cessna model is going to be lighted by our customized lights. We will load
    /// it from file before creating the light sources:
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("cessna.osg" );
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( model.get() );

    /// Now it's time to construct two light source nodes and put them at different positions in the scene:
    osg::Node* light0 = createLightSource(0, osg::Vec3(-20.0f,0.0f,0.0f), osg::Vec4(1.0f,1.0f,0.0f,1.0f));
    osg::Node* light1 = createLightSource(1, osg::Vec3(0.0f,-20.0f,0.0f), osg::Vec4(0.0f,1.0f,1.0f,1.0f));

    /// The light numbers 0 and 1 are used here. So we will turn on modes GL_LIGHT0
    /// and GL_LIGHT1 of the root node, which means that all nodes in the scene graph
    /// could benefit from the two warm light sources:
    root->getOrCreateStateSet()->setMode( GL_LIGHT0, osg::StateAttribute::ON );
    root->getOrCreateStateSet()->setMode( GL_LIGHT1, osg::StateAttribute::ON );
    root->addChild( light0 );
    root->addChild( light1 );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();

    /// You will figure out that one side of the Cessna is lighted in yellow, and its front is
    /// caught by a cyan light. That is exactly what we want in the example source code!
}
