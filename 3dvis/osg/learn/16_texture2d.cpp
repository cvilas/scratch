/// The most common texture mapping technique is 2D texture mapping. This accepts a 2D image
/// as the texture and maps it onto one or more geometry surfaces. The osg::Texture2D class is
/// used here as a texture attribute of a specific texture mapping unit.

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main()
{
    /// We will quickly create a quad and call the setTexCoordArray() method to
    /// bind texture coordinates per vertex. The texture coordinate array only affects the
    /// texture unit 0 in this example, but it is always possible to share arrays among units:
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back( osg::Vec3(-0.5f, 0.0f,-0.5f) );
    vertices->push_back( osg::Vec3( 0.5f, 0.0f,-0.5f) );
    vertices->push_back( osg::Vec3( 0.5f, 0.0f, 0.5f) );
    vertices->push_back( osg::Vec3(-0.5f, 0.0f, 0.5f) );

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0.0f,-1.0f, 0.0f) );

    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    texcoords->push_back( osg::Vec2(0.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.0f) );

    osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
    quad->setVertexArray( vertices.get() );
    quad->setNormalArray( normals.get() );
    quad->setNormalBinding( osg::Geometry::BIND_OVERALL );
    quad->setTexCoordArray( 0, texcoords.get() );
    quad->addPrimitiveSet( new osg::DrawArrays(GL_QUADS, 0, 4) );

    /// We will load an image from the disk and assign it to the 2D texture object. The file
    /// format .rgb is developed by SGI and is commonly used for storing 2D textures:
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( "Images/lz.rgb" );
    texture->setImage( image.get() );

    /// Add the quad to an osg::Geode node, and then add the texture attribute to the
    /// state set. Be careful to set the attribute to the same texture mapping unit as the
    /// texture coordinate array:
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( quad.get() );
    root->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
