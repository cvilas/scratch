#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include "custom_object_view.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  QString extraImportPath(QStringLiteral("%1/../pluginlib"));
  QString path = extraImportPath.arg(QGuiApplication::applicationDirPath());
  engine.addImportPath(path);
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  /// Find the object by name from the qml graph and manipulate it
  /// Accessing qml objects directly by name is not the recommended approach, however
  CustomObjectView *uiObject = engine.rootObjects()[0]->findChild<CustomObjectView*>("myCustomObject");

  uiObject->setName("test object");
  /// connect a timer and just periodically change the value. You should see the gui update
  QTimer timer;
  timer.setSingleShot(false);
  QObject::connect(&timer, SIGNAL(timeout()), uiObject, SLOT(incrementCount()));
  timer.start(1000);

  return app.exec();
}
