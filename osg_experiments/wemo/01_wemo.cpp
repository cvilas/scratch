#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>

#include <hive/robohive.h>
#include <hive/types/navigation/odometry.h>

//---------------------------------------------------------------------------------------------------------------------
void onOdometry(const hive::uuid& src, const std::exception_ptr& ex, const hive::types::navigation::Odometry& odo)
//---------------------------------------------------------------------------------------------------------------------
{
    /// Receive odometry message from DDS, package it and publish it to ROS

    (void)src;

    if (ex != nullptr)
    {
      //exception_ptr_ = ex;
      return;
    }

}

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
//---------------------------------------------------------------------------------------------------------------------
{
    //hive::init(argc, argv);

    osg::ref_ptr<osg::Node> wheel = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_wheel.osgt" );
    osg::ref_ptr<osg::Node> body = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_body.osgt" );

    /// PositionAttitudeTransform is easier to use than MatrixTransform
    osg::ref_ptr<osg::PositionAttitudeTransform> tr_body = new osg::PositionAttitudeTransform;
    tr_body->setPosition( osg::Vec3d(0.0f, 0.0f, 0.0f) );
    tr_body->addChild( body.get() );

    osg::ref_ptr<osg::MatrixTransform> tr_wheel1 = new osg::MatrixTransform;
    tr_wheel1->setMatrix( osg::Matrix::translate(.350f, .0f, 0.350f) );
    tr_wheel1->addChild( wheel.get() );

    osg::ref_ptr<osg::MatrixTransform> tr_wheel2 = new osg::MatrixTransform;
    tr_wheel2->setMatrix( osg::Matrix::translate(-.350f, .0f, 0.350f) );
    tr_wheel2->addChild( wheel.get() );

    osg::ref_ptr<osg::MatrixTransform> tr_wheel3 = new osg::MatrixTransform;
    tr_wheel3->setMatrix( osg::Matrix::translate(.350f, .0f, -0.350f) );
    tr_wheel3->addChild( wheel.get() );

    osg::ref_ptr<osg::MatrixTransform> tr_wheel4 = new osg::MatrixTransform;
    tr_wheel4->setMatrix( osg::Matrix::translate(-.350f, .0f, -0.350f) );
    tr_wheel4->addChild( wheel.get() );

    /// Add the two transformation nodes to the root node and start the viewer:
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( tr_body.get() );
    tr_body->addChild( tr_wheel1.get() );
    tr_body->addChild( tr_wheel2.get() );
    tr_body->addChild( tr_wheel3.get() );
    tr_body->addChild( tr_wheel4.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
