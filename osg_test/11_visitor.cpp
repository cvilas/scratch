/// User applications may always search the loaded scene graph for nodes of interest after
/// loading a model file. For example, we might like to take charge of the transformation or
/// visibility of the loaded model if the root node is osg::Transform or osg::Switch. We
/// might also be interested in collecting all transformation nodes at the joints of a skeleton,
/// which can be used to perform character animations later.
/// The analysis of the loaded model structure is important in that case. We will implement an
/// information printing visitor here, which prints the basic information of visited nodes and
/// arranges them in a tree structure.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <iostream>

/// Declare the InfoVisitor class, and define the necessary virtual methods. We only
/// handle leaf nodes and common osg::Node objects. The inline function spaces()
/// is used for printing spaces before node information, to indicate its level in the tree structure:
class InfoVisitor : public osg::NodeVisitor
{
public:
    InfoVisitor() : _level(0)
    {
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    }
    std::string spaces()
    {
        return std::string(_level*2, ' ');
    }

    void apply( osg::Node& node ) override;
    void apply( osg::Geode& geode ) override;
protected:
    unsigned int _level;
};

/// We will introduce two methods className() and libraryName(), both of
/// which return const char* values, for instance, "Node" as the class name and "osg"
/// as the library name. There is no trick in re-implementing these two methods for
/// different classes. The META_Object and META_Node macro definitions will do the
/// work internally:

void InfoVisitor::apply( osg::Node& node )
{
    auto name = node.getName();
    std::cout << spaces() << node.libraryName() << "::" << node.className()
              << " (name: " << (name.empty()?"noname":name) << ")" << std::endl;

    _level++;
    traverse( node );
    _level--;
}

/// The implementation of the apply() overloaded method with the osg::Geode&
/// parameter is slightly different from the previous one. It iterates all attached
/// drawables of the osg::Geode node and prints their information, too. Be aware of
/// the calling time of traverse() here, which ensures that the level of each node in
/// the tree is correct.

void InfoVisitor::apply( osg::Geode& geode )
{
    std::cout << spaces() << geode.libraryName() << "::" << geode.className() << std::endl;
    _level++;
    for ( unsigned int i=0; i<geode.getNumDrawables(); ++i )
    {
        osg::Drawable* drawable = geode.getDrawable(i);
        auto name = drawable->getName();
        std::cout << spaces() << drawable->libraryName() << "::" << drawable->className()
                  << " (name: " << (name.empty()?"noname":name) << ")" << std::endl;
    }

    traverse( geode );
    _level--;
}

/// Try running with 'cessnafire.osg' as the command line parameter
int main(int argc, char* argv[])
{
    /// read a file from command line arguments:
    osg::ArgumentParser arguments( &argc, argv );
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles( arguments );
    if ( !root )
    {
        OSG_FATAL << arguments.getApplicationName() <<": No data loaded." << std::endl;
        return -1;
    }

    /// Use the customized InfoVisitor to visit the loaded model now. You will have
    /// noticed that the setTraversalMode() method is called in the constructor of the
    /// visitor in order to enable the traversal of all of its children:
    InfoVisitor infoVisitor;
    root->accept( infoVisitor );

    /// Start the viewer or not. This depends on your opinion, because our visitor has
    /// already finished its mission:
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
