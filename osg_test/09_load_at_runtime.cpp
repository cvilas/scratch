/// We are going to demonstrate the loading of a model file by using the osg::ProxyNode. The
/// proxy will record the filename of the original model, and defer loading it until the viewer is
/// running and sending corresponding requests.
#include <osg/ProxyNode>
#include <osgViewer/Viewer>

int main()
{
    /// Instead of just loading model files as child nodes, we will set a filename to the
    /// specified index of children. This is similar to the insertChild() method, which
    /// puts a node into the specified position of the children list, but the list will not be
    /// filled until the dynamic loading process has finished.
    osg::ref_ptr<osg::ProxyNode> root = new osg::ProxyNode;
    root->setFileName( 0, "cow.osg" );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
