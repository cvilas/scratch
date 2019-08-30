#include "window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	Window window;
  window.resize(640,480);
  window.show();
	return app.exec();
}
