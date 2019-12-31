#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osg::setNotifyLevel( osg::NOTICE );

    osg::ref_ptr<osg::Node> root = osgDB::readNodeFile("cessna.osg");
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get());

    OSG_ALWAYS << arguments.getApplicationName() << ": Scene set" << std::endl;

    return viewer.run();
}
