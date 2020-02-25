#include <QGuiApplication>
#include <QSceneLoader>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
//#include <Qt3DCore/QAspectEngine>

//#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DRender/QCamera>
//#include <Qt3DRender/QMaterial>
//#include <Qt3DRender/QMesh>

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
//#include <Qt3DExtras/QPhongMaterial>

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
  Qt3DCore::QEntity* getScene();
  auto ok() -> bool;

private:
  void initScene();
  void initHive();
  void onOdometry(const hive::uuid& src, const std::exception_ptr& ex, const hive::types::Odometry& odo);

private:
  using OdometryReader = hive::dds::Reader<hive::types::Odometry>;

  std::exception_ptr exception_ptr_{};
  Qt3DCore::QEntity* scene_root_{nullptr};
  Qt3DCore::QTransform* robot_tr_body_{nullptr};
  Qt3DCore::QTransform* tr_wheel1_{nullptr};
  Qt3DCore::QTransform* tr_wheel2_{nullptr};
  Qt3DCore::QTransform* tr_wheel3_{nullptr};
  Qt3DCore::QTransform* tr_wheel4_{nullptr};
  std::unique_ptr<hive::dds::Participant> participant_{ nullptr };
  std::unique_ptr<hive::logger::DDSLogAppender> log_appender_{ nullptr };
  std::unique_ptr<OdometryReader> odo_sub_{ nullptr };
  //hive::mobility::WemoMotionModel motion_model_;
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
    scene_root_ = new Qt3DCore::QEntity;

    auto *body = new Qt3DRender::QSceneLoader(scene_root_);
    body->setSource(QUrl("qrc:/model/body.gltf"));
    robot_tr_body_ = new Qt3DCore::QTransform;
    robot_tr_body_->setTranslation(QVector3D(0.0, 0.0, 0.0));

    scene_root_->addComponent(body);
    scene_root_->addComponent(robot_tr_body_);

    auto *wheel1 = new Qt3DRender::QSceneLoader(scene_root_);
    wheel1->setSource(QUrl("qrc:/model/wheel1.gltf"));
    scene_root_->addComponent(wheel1);

/*********
    Qt3DCore::QTransform* transform = new Qt3DCore::QTransform;
//    transform->setScale3D(QVector3D(1.5, 1, 0.5));
    transform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), 45.f ));

    Qt3DRender::QMaterial* material = new Qt3DExtras::QPhongMaterial(scene_root_);

    torus->addComponent(mesh);
    torus->addComponent(transform);
    torus->addComponent(material);
*/

    /*******************
  osg::ref_ptr<osg::Node> wheel1 = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_wheel0.osgt" );
  osg::ref_ptr<osg::Node> wheel0 = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_wheel1.osgt" );
  osg::ref_ptr<osg::Node> body = osgDB::readNodeFile("/home/vilas/projects/mine/scratch/osg_experiments/wemo/model/wemo_body.osgt" );

  robot_tr_body_ = new osg::PositionAttitudeTransform;
  robot_tr_body_->setPosition( osg::Vec3d(0.0, 0.0, 0.0) );
  robot_tr_body_->addChild( body.get() );

  tr_wheel1_ = new osg::PositionAttitudeTransform;
  tr_wheel1_->setPosition(osg::Vec3d(350, 350, 102.4));
  osg::ref_ptr<osg::MatrixTransform> offset1 = new osg::MatrixTransform;
  offset1->setMatrix(osg::Matrix::translate(0,0,-152.4));
  offset1->addChild(wheel0.get());
  tr_wheel1_->addChild( offset1.get() );

  tr_wheel2_ = new osg::PositionAttitudeTransform;
  tr_wheel2_->setPosition(osg::Vec3d(-350, 350, 102.4) );
  osg::ref_ptr<osg::MatrixTransform> offset2 = new osg::MatrixTransform;
  offset2->setMatrix(osg::Matrix::translate(0,0,-152.4));
  offset2->addChild(wheel1.get());
  tr_wheel2_->addChild( offset2.get() );

  tr_wheel3_ = new osg::PositionAttitudeTransform;
  tr_wheel3_->setPosition(osg::Vec3d(-350, -350, 102.4) );
  osg::ref_ptr<osg::MatrixTransform> offset3 = new osg::MatrixTransform;
  offset3->setMatrix(osg::Matrix::translate(0,0,-152.4));
  offset3->addChild(wheel0.get());
  tr_wheel3_->addChild( offset3.get() );

  tr_wheel4_ = new osg::PositionAttitudeTransform;
  tr_wheel4_->setPosition(osg::Vec3d(350, -350, 102.4) );
  osg::ref_ptr<osg::MatrixTransform> offset4 = new osg::MatrixTransform;
  offset4->setMatrix(osg::Matrix::translate(0,0,-152.4));
  offset4->addChild(wheel1.get());
  tr_wheel4_->addChild( offset4.get() );

  robot_tr_body_->addChild(tr_wheel1_.get());
  robot_tr_body_->addChild(tr_wheel2_.get());
  robot_tr_body_->addChild(tr_wheel3_.get());
  robot_tr_body_->addChild(tr_wheel4_.get());

  /// Add the two transformation nodes to the root node and start the viewer:
  scene_root_ = new osg::Group;
  scene_root_->addChild(robot_tr_body_.get());*/
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
Qt3DCore::QEntity* Visualiser::getScene()
//---------------------------------------------------------------------------------------------------------------------
{
  return scene_root_;
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
void Visualiser::onOdometry(const hive::uuid& src, const std::exception_ptr& ex, const hive::types::Odometry& odo)
//---------------------------------------------------------------------------------------------------------------------
{
  (void)src;
  if (ex != nullptr)
  {
    exception_ptr_ = ex;
    return;
  }

  const QVector3D pos(
    odo.uncertain_pose.pose.position.x(),
        odo.uncertain_pose.pose.position.y(),
        odo.uncertain_pose.pose.position.z());

  const QQuaternion rot(odo.uncertain_pose.pose.orientation.w(),
    odo.uncertain_pose.pose.orientation.x(),
        odo.uncertain_pose.pose.orientation.y(),
        odo.uncertain_pose.pose.orientation.z());

  robot_tr_body_->setTranslation(pos * 1000.);
  robot_tr_body_->setRotation(rot);

  /// \todo: get rid of kinematic model and drive the wheels directly from hardware messages
/*
  static Eigen::Vector4d wheels{0,0,0,0};
  static auto last_time = hive::timing::secondsSinceEpoch();
  const auto now = hive::timing::secondsSinceEpoch();
  const auto dt = now - last_time;
  last_time = now;
  const auto wheel_speeds = motion_model_.computeWheelDelta(odo.uncertain_twist.twist.linear.x(),
                                                            odo.uncertain_twist.twist.linear.y(),
                                                            odo.uncertain_twist.twist.angular.z());
  wheels += wheel_speeds * dt;
  tr_wheel1_->setAttitude(osg::Quat(wheels[0], osg::Vec3d(0,1,0)));
  tr_wheel2_->setAttitude(osg::Quat(wheels[1], osg::Vec3d(0,1,0)));
  tr_wheel3_->setAttitude(osg::Quat(wheels[2], osg::Vec3d(0,-1,0)));
  tr_wheel4_->setAttitude(osg::Quat(wheels[3], osg::Vec3d(0,-1,0)));
*/
}

//=====================================================================================================================
int main(int argc, char* argv[])
//=====================================================================================================================
{
    try
    {
      //hive::init(argc, argv);

      Visualiser vis;

      QGuiApplication app(argc, argv);
      Qt3DExtras::Qt3DWindow view;
      Qt3DCore::QEntity* scene = vis.getScene();

      // camera
      Qt3DRender::QCamera *camera = view.camera();
      camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.001f, 10000.0f);
      //camera->setPosition(QVector3D(384, 210, -251.0f));
      //camera->setViewCenter(QVector3D(0, 1, 0));

      // manipulator
      Qt3DExtras::QOrbitCameraController* manipulator = new Qt3DExtras::QOrbitCameraController(scene);
      manipulator->setLinearSpeed(50.f);
      manipulator->setLookSpeed(180.f);
      manipulator->setCamera(camera);

      view.setRootEntity(scene);
      view.show();

      while (vis.ok())
      {
          /// \todo handle qapp exit
        app.processEvents();
      }

      return 0;

    } catch (const std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return -1;
    }

}

