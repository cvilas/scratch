#include <GL/glut.h>
#include <osg/Drawable>
#include <osg/Geode>
#include <osgViewer/Viewer>

/// To implement own Drawable, overload following virtual methods in the osg::Drawable class:
/// - computeBoundingBox() to compute the bounding box around the geometry, which will be used in
/// the view frustum culling process to decide whether to cull the geometry or not
/// - drawImplementation() to draw the geometry with OSG and OpenGL calls
/// The TeapotDrawable class just draws the classic teapot model from glut library
class TeapotDrawable : public osg::Drawable
{
public:
    TeapotDrawable( float size=1.0f )
        : _size(size)
    {
    }

    /// copy constructor to help instantiate our TeapotDrawable class
    TeapotDrawable( const TeapotDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
        : osg::Drawable(copy, copyop), _size(copy._size) {}

    /// implements some basic properties of the class.
    META_Object( osg, TeapotDrawable );

    /// calculate bounding box
    osg::BoundingBox computeBoundingBox() const override
    {
        osg::Vec3 min(-_size,-_size,-_size), max(_size, _size, _size);
        return osg::BoundingBox(min, max);
    }

    /// just draw the glut teapot
    void drawImplementation( osg::RenderInfo& ) const override
    {
        glFrontFace( GL_CW );
        glutSolidTeapot( _size );
        glFrontFace( GL_CCW );
    }

protected:
    float _size;
};

int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // since we are using glut

    // add our custom drawable
    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    root->addDrawable( new TeapotDrawable(1.0f) );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
