#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

/// Shows
/// - how to use command line argnuments parser
/// - streaming to builtin logger
/// - basic viewer

int main( int argc, char** argv )
{
    osg::setNotifyLevel( osg::NOTICE );

    std::string model_file = "cessna.osg"; //!< default model to render
    osg::ArgumentParser arguments( &argc, argv );
    arguments.read( "--model", model_file ); //!< read command line for model to render

    osg::ref_ptr<osg::Node> root = osgDB::readNodeFile(model_file);
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get());

    OSG_ALWAYS << arguments.getApplicationName() << ": Scene set" << std::endl;

    return viewer.run();
}
