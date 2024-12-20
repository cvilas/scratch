#include "widget.h"
#include "line_graph.h"
#include <QQmlContext>

Widget::Widget(QWidget *parent) : QWidget(parent) {
    Q_UNUSED(parent);
    line_graph_ = new LineGraph;
    quick_widget_ = new QQuickWidget;
    widget_ = new QWidget;
    layout_ = new QGridLayout(widget_);
    layout_->addWidget(quick_widget_);

    QString extraImportPath(QStringLiteral("%1/../../../%2"));
    quick_widget_->engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));
    QQmlContext *context = quick_widget_->engine()->rootContext();
    context->setContextProperty("lineGraph", line_graph_);
    quick_widget_->setSource(QUrl("qrc:/qml/graph/main.qml"));
    quick_widget_->setResizeMode(QQuickWidget::SizeRootObjectToView);
}

Widget::~Widget() {
    delete quick_widget_;
    delete layout_;
    delete widget_;
    delete line_graph_;
}

auto Widget::containerWidget() const -> QWidget* {
    return widget_;
}