#include <osg/LOD>
#include <osgDB/ReadFile>
#include <osgUtil/Simplifier>
#include <osgViewer/Viewer>

/// Render the same model at different levels of detail (LOD)
int main()
{
    // We would like to build three levels of model details. First, we need to create three
    // copies of the original model. It is OK to read the Cessna from the file three times,
    // but here a clone() method is called to duplicate the loaded model for immediate uses:
    osg::ref_ptr<osg::Node> modelL3 = osgDB::readNodeFile("cessna.osg");
    osg::ref_ptr<osg::Node> modelL2 = dynamic_cast<osg::Node*>(modelL3->clone(osg::CopyOp::DEEP_COPY_ALL) );
    osg::ref_ptr<osg::Node> modelL1 = dynamic_cast<osg::Node*>(modelL3->clone(osg::CopyOp::DEEP_COPY_ALL) );

    // Level three will be the original Cessna, which has the maximum number of polygons for
    // close-up viewing. Level two has fewer polygons to show, and level one will be the least detailed,
    // which is displayed only at a very far distance.
    // The osgUtil::Simplifier class is used here to reduce the vertices and faces. We apply the
    // setSampleRatio() method to the level 1 and level 2 models with different values, which results in
    // different simplifying rates:
    osgUtil::Simplifier simplifier;
    simplifier.setSampleRatio( 0.5 );
    modelL2->accept( simplifier );

    simplifier.setSampleRatio( 0.1 );
    modelL1->accept( simplifier );

    // Add level models to the LOD node and set their visible range in descending order. Don't make
    // overlapping ranges when you are configuring minimum and maximum range values with the addChild()
    // or setRange() method, otherwise there will be more than one level of model shown at the same position,
    // which results in incorrect behaviors:
    //
    // The osg::PagedLOD node also inherits methods of osg::LOD, but dynamically loads and unloads levels of
    // detail in order to avoid overloading the graphics pipeline and keep the rendering process as smooth as possible.
    osg::ref_ptr<osg::LOD> root = new osg::LOD;
    root->addChild( modelL1.get(), 200.0f, FLT_MAX );
    root->addChild( modelL2.get(), 50.0f, 200.0f );
    root->addChild( modelL3.get(), 0.0f, 50.0f );

    // Start the viewer. The application will need a little more time to compute and reduce model faces this time:
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();

    // The Cessna model shows. Try pressing and holding the right mouse button to zoom in and out. You will find
    // that the model is still well-represented when looking close. However, the model is slightly simpler when
    // viewing from far distances. This difference will not affect the rendering result a lot, but will enhance the
    // system's efficiency if properly used.

}
