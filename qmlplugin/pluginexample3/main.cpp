#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include "model.h"


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

  Model model;
  auto item = new CustomObjectView;
  item->setName("test object 1");
  item->setValue(1);
  model.insert(item);

  auto item2 = new CustomObjectView;
  item2->setName("test object 2");
  item2->setValue(2);
  model.insert(item2);

  engine.rootContext()->setContextProperty("MyModel", &model);

  QTimer timer;
  timer.setSingleShot(false);
  QObject::connect(&timer, SIGNAL(timeout()), &model, SLOT(testModifyList()));
  timer.start(1000);

  return app.exec();
}
