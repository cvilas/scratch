#include <QApplication>
#include <QtQml/qqmlengine.h>

#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget widget;
    widget.containerWidget()->show();
    return app.exec();
}

