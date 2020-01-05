#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <hive/robohive.h>
#include <hive/types/navigation/odometry.h>
#include <hive/dds/participant.h>
#include <hive/dds/reader.h>
#include <hive/logger/dds_log_appender.h>
#include <hive/mobility/platform_odometry_topic.h>
#include <hive/wemo/wemo_motion_model.h>

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
  osg::ref_ptr<osg::PositionAttitudeTransform> tr_wheel1_{nullptr};
  osg::ref_ptr<osg::PositionAttitudeTransform> tr_wheel2_{nullptr};
  osg::ref_ptr<osg::PositionAttitudeTransform> tr_wheel3_{nullptr};
  osg::ref_ptr<osg::PositionAttitudeTransform> tr_wheel4_{nullptr};
  std::unique_ptr<hive::dds::Participant> participant_{ nullptr };
  std::unique_ptr<hive::logger::DDSLogAppender> log_appender_{ nullptr };
  std::unique_ptr<OdometryReader> odo_sub_{ nullptr };
  hive::mobility::WemoMotionModel motion_model_;
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

  // rotate to align body frame with our body frame convention (X:forward, Y:port, Z:up)
  osg::ref_ptr<osg::MatrixTransform> rotate_body = new osg::MatrixTransform(osg::Matrix::rotate(-M_PI/2, 1, 0, 0));
  rotate_body->addChild(body.get());

  /// \todo:
  /// - wheel positions should be read from WemoMotionModel
  /// - we need two different wheel models for left and right wheels
  /// - set wheel position and orientation properly
  /// - set transform to rotate wheel about axis

  tr_wheel1_ = new osg::PositionAttitudeTransform;
  tr_wheel1_->setPosition(osg::Vec3d(0.350, 0.0, 0.350));
  tr_wheel1_->addChild( wheel.get() );

  tr_wheel2_ = new osg::PositionAttitudeTransform;
  tr_wheel2_->setPosition(osg::Vec3d(-0.350, 0.0, 0.350) );
  tr_wheel2_->addChild( wheel.get() );

  tr_wheel3_ = new osg::PositionAttitudeTransform;
  tr_wheel3_->setPosition(osg::Vec3d(-0.350, 0.0, -0.350) );
  tr_wheel3_->addChild( wheel.get() );

  tr_wheel4_ = new osg::PositionAttitudeTransform;
  tr_wheel4_->setPosition(osg::Vec3d(0.350, 0.0, -0.350) );
  tr_wheel4_->addChild( wheel.get() );

  rotate_body->addChild( tr_wheel1_.get() );
  rotate_body->addChild( tr_wheel2_.get() );
  rotate_body->addChild( tr_wheel3_.get() );
  rotate_body->addChild( tr_wheel4_.get() );

  robot_tr_body_ = new osg::PositionAttitudeTransform;
  robot_tr_body_->setPosition( osg::Vec3d(0.0, 0.0, 0.0) );
  robot_tr_body_->addChild( rotate_body.get() );

  /// Add the two transformation nodes to the root node and start the viewer:
  osg_root_ = new osg::Group;
  osg_root_->addChild( robot_tr_body_.get() );
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

  const osg::Vec3d pos{
    odo.uncertain_pose.pose.position.x(),
        odo.uncertain_pose.pose.position.y(),
        odo.uncertain_pose.pose.position.z()};

  const osg::Quat rot{
    odo.uncertain_pose.pose.orientation.x(),
        odo.uncertain_pose.pose.orientation.y(),
        odo.uncertain_pose.pose.orientation.z(),
        odo.uncertain_pose.pose.orientation.w()};

  robot_tr_body_->setPosition(pos);
  robot_tr_body_->setAttitude(rot);

  static Eigen::Vector4d wheels{0,0,0,0};
  static auto last_time = hive::timing::secondsSinceEpoch();
  const auto now = hive::timing::secondsSinceEpoch();
  const auto dt = now - last_time;
  last_time = now;
  const auto wheel_speeds = motion_model_.computeWheelDelta(odo.uncertain_twist.twist.linear.x(),
                                                            odo.uncertain_twist.twist.linear.y(),
                                                            odo.uncertain_twist.twist.angular.z());
  /// \todo
  /// - set wheel positions correctly
  /// - set wheel orientation so that rollers point in the right direction
  /// - set up animation to rotate the wheels at speed above

  wheels += wheel_speeds * dt;
  tr_wheel1_->setAttitude(osg::Quat(wheels[0], osg::Vec3d(0,0,1)));
  tr_wheel2_->setAttitude(osg::Quat(wheels[1], osg::Vec3d(0,0,1)));
  tr_wheel3_->setAttitude(osg::Quat(wheels[2], osg::Vec3d(0,0,-1)));
  tr_wheel4_->setAttitude(osg::Quat(wheels[3], osg::Vec3d(0,0,-1)));

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
    viewer.setSceneData( vis.getScene() );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

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
