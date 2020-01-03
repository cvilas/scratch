/// The most important step in customizing a node and extending new features is to override
/// the virtual method traverse(). This method is called every frame by the OSG rendering
/// backend. The traverse() method has an input parameter, osg::NodeVisitor&, which
/// actually indicates the type of traversals (update, event, or cull). Most OSG NodeKits override
/// traverse() to implement their own functionalities, along with some other exclusive
/// attributes and methods.
/// Note that overriding the traverse() method is a bit dangerous sometimes, because it affects
/// the traversing process and may lead to the incorrect rendering of results if developers are not
/// careful enough. It is also a little awkward if you want to add the same new feature to multiple
/// node types by extending each node type to a new customized class. In these cases, consider
/// using node callbacks instead, which will be discussed in Chapter 8, Animating Scene Objects.
///
/// The osg::Switch class can display specified child nodes while hiding others. It could be
/// used to represent the animation states of various objects, for instance, traffic lights.
/// However, a typical osg::Switch node is not able to automatically switch between children
/// at different times. Based on this idea, we will develop a new AnimatingSwitch node,
/// which will display its children at one time, and reverse the switch states according to a
/// user-defined internal counter.

#include <osg/Switch>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

/// Declare the AnimatingSwitch class. This will be derived from the osg::Switch
/// class to take advantage of the setValue() method. We also make use of an
/// OSG macro definition, META_Node, which is a little similar to the META_Object
/// introduced in the last chapter, to define basic properties (library and class name)
/// of a node:
class AnimatingSwitch : public osg::Switch
{
public:
    AnimatingSwitch() : osg::Switch(), _count(0) {}
    AnimatingSwitch( const AnimatingSwitch& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
        : osg::Switch(copy, copyop), _count(copy._count) {}
    META_Node( osg, AnimatingSwitch );
    virtual void traverse( osg::NodeVisitor& nv );
protected:
    unsigned int _count;
};

/// In the traverse() implementation, we will increase the internal counter and see
/// if it reaches a multiple of 60, and reverse the states of the first and second child nodes:
void AnimatingSwitch::traverse( osg::NodeVisitor& nv )
{
    if ( !((++_count)%60) )
    {
        setValue( 0, !getValue(0) );
        setValue( 1, !getValue(1) );
    }
    osg::Switch::traverse( nv );
}

int main()
{
    /// Read the Cessna model and the afire model again and add them to the customized AnimatingSwitch instance:
    osg::ref_ptr<osg::Node> model1= osgDB::readNodeFile("cessna.osg");
    osg::ref_ptr<osg::Node> model2= osgDB::readNodeFile("cessnafire.osg");

    osg::ref_ptr<AnimatingSwitch> root = new AnimatingSwitch;
    root->addChild( model1.get(), true );
    root->addChild( model2.get(), false );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();

    /// Because the hardware refresh rate is often at 60 Hz, the if condition in
    /// traverse() will become true once per second, which achieves the animation.
    /// Then you will see the Cessna is intact in the first second, and afire and smoking
    /// in the next, acting in cycles:
}
