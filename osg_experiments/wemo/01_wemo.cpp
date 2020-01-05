#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>

#include <hive/robohive.h>
#include <hive/types/navigation/odometry.h>
#include <hive/dds/participant.h>
#include <hive/dds/reader.h>
#include <hive/logger/dds_log_appender.h>
#include <hive/mobility/platform_odometry_topic.h>

//=====================================================================================================================
// Demo class to set up visualisation
//=====================================================================================================================
class Visualiser
{
public:
  Visualiser();
  ~Visualiser() = default;
  osg::Node* getScene();
  auto ok() -> bool;

private:
  void initScene();
  void initHive();
  void onOdometry(const hive::uuid& src, const std::exception_ptr& ex, const hive::types::navigation::Odometry& odo);

private:
  using OdometryReader = hive::dds::Reader<hive::types::navigation::Odometry>;

  std::exception_ptr exception_ptr_{};
  osg::ref_ptr<osg::Group> osg_root_{nullptr};
  osg::ref_ptr<osg::PositionAttitudeTransform> robot_tr_body_{nullptr};
  std::unique_ptr<hive::dds::Participant> participant_{ nullptr };
  std::unique_ptr<hive::logger::DDSLogAppender> log_appender_{ nullptr };
  std::unique_ptr<OdometryReader> odo_sub_{ nullptr };
};

//---------------------------------------------------------------------------------------------------------------------
Visualiser::Visualiser()
//---------------------------------------------------------------------------------------------------------------------
{
  initScene();
  initHive();
}

//---------------------------------------------------------------------------------------------------------------------
void Visualiser::initScene()
//---------------------------------------------------------------------------------------------------------------------
{
  osg::ref_ptr<osg::Node> wheel = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_wheel.osgt" );
  osg::ref_ptr<osg::Node> body = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_body.osgt" );

  /// PositionAttitudeTransform is easier to use than MatrixTransform
  robot_tr_body_ = new osg::PositionAttitudeTransform;
  robot_tr_body_->setPosition( osg::Vec3d(0.0, 0.0, 0.0) );
  robot_tr_body_->addChild( body.get() );

  osg::ref_ptr<osg::MatrixTransform> tr_wheel1 = new osg::MatrixTransform;
  tr_wheel1->setMatrix( osg::Matrix::translate(.350, .0, 0.350) );
  tr_wheel1->addChild( wheel.get() );

  osg::ref_ptr<osg::MatrixTransform> tr_wheel2 = new osg::MatrixTransform;
  tr_wheel2->setMatrix( osg::Matrix::translate(-.350, .0, 0.350) );
  tr_wheel2->addChild( wheel.get() );

  osg::ref_ptr<osg::MatrixTransform> tr_wheel3 = new osg::MatrixTransform;
  tr_wheel3->setMatrix( osg::Matrix::translate(.350, .0, -0.350) );
  tr_wheel3->addChild( wheel.get() );

  osg::ref_ptr<osg::MatrixTransform> tr_wheel4 = new osg::MatrixTransform;
  tr_wheel4->setMatrix( osg::Matrix::translate(-.350, .0, -0.350) );
  tr_wheel4->addChild( wheel.get() );

  /// Add the two transformation nodes to the root node and start the viewer:
  osg_root_ = new osg::Group;
  osg_root_->addChild( robot_tr_body_.get() );
  robot_tr_body_->addChild( tr_wheel1.get() );
  robot_tr_body_->addChild( tr_wheel2.get() );
  robot_tr_body_->addChild( tr_wheel3.get() );
  robot_tr_body_->addChild( tr_wheel4.get() );
}

//---------------------------------------------------------------------------------------------------------------------
void Visualiser::initHive()
//---------------------------------------------------------------------------------------------------------------------
{
  hive::logger::setThreshold(hive::logger::Severity::Debug);
  participant_ = std::make_unique<hive::dds::Participant>(hive::ddsDomain(), "wemo visualiser test",
                                                          std::chrono::seconds(1),
                                                          std::vector<std::byte>(),
                                                          nullptr, nullptr, nullptr);

  odo_sub_ = std::make_unique<OdometryReader>(*participant_, hive::mobility::PlatformOdometryTopic(),
                                              std::bind(&Visualiser::onOdometry, this, std::placeholders::_1,
                                                        std::placeholders::_2, std::placeholders::_3));
}

//---------------------------------------------------------------------------------------------------------------------
osg::Node* Visualiser::getScene()
//---------------------------------------------------------------------------------------------------------------------
{
  return osg_root_.release();
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualiser::ok() -> bool
//---------------------------------------------------------------------------------------------------------------------
{
  if (exception_ptr_ != nullptr)
  {
    std::rethrow_exception(exception_ptr_);
  }

  return true;
}

//---------------------------------------------------------------------------------------------------------------------
void Visualiser::onOdometry(const hive::uuid& src, const std::exception_ptr& ex, const hive::types::navigation::Odometry& odo)
//---------------------------------------------------------------------------------------------------------------------
{
  (void)src;
  if (ex != nullptr)
  {
    exception_ptr_ = ex;
    return;
  }

  osg::Vec3d pos{
    odo.uncertain_pose.pose.position.x(),
        odo.uncertain_pose.pose.position.y(),
        odo.uncertain_pose.pose.position.z()};
  robot_tr_body_->setPosition(pos);

}

//=====================================================================================================================
int main(int argc, const char* argv[])
//=====================================================================================================================
{
  try
  {
    hive::init(argc, argv);

    Visualiser vis;

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(0,0,640,480);
    osg::ref_ptr<osg::Node> scene = vis.getScene();
    viewer.setSceneData( scene );
    return viewer.run();

    while (!viewer.done() && vis.ok())
    {
      viewer.frame();
    }
    return 0;
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << "\n";
    return -1;
  }
}
