#include "wrapper.h"

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

//=====================================================================================================================
int main(int argc, char* argv[])
//=====================================================================================================================
{
  QGuiApplication app(argc, argv);
  QQuickView viewer;

  //QString extraImportPath(QStringLiteral("%1/../../../%2"));
  //viewer.engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));

  // set path to find the plugin
  QString extraImportPath(QStringLiteral("%1/../pluginlib"));
  QString path = extraImportPath.arg(QGuiApplication::applicationDirPath());
  viewer.engine()->addImportPath(path);

  // create list of my custom objects
  Wrapper wrapper;

  // and set the list as model
  QQmlContext *ctxt = viewer.rootContext();
  ctxt->setContextProperty("wrapper", &wrapper);

  // set up connection
  QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);
  viewer.setTitle(QStringLiteral("Custom Plugin Demo"));

  viewer.setResizeMode(QQuickView::SizeRootObjectToView);
  viewer.setSource(QUrl("qrc:/qml/view.qml"));
  viewer.show();

  return QGuiApplication::exec();
}
