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

    quick_widget_->rootContext()->setContextProperty("lineGraph", line_graph_);
    quick_widget_->rootContext()->setContextProperty("lineSeries", line_graph_->lineSeries());
    quick_widget_->setSource(QUrl("qrc:/main.qml"));
   if (quick_widget_->status() == QQuickWidget::Error) {
        qWarning() << "QML Errors:";
        for (const auto &error : quick_widget_->errors()) {
            qWarning() << error. toString();
        }
    }
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