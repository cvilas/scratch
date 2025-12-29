#pragma once

#include <QWidget>
#include <QQuickWidget>
#include <QGridLayout>

class LineGraph;

class Widget : public QWidget {
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    auto containerWidget() const -> QWidget*;
private:
    QWidget* widget_;
    QQuickWidget* quick_widget_;
    QGridLayout* layout_;
    LineGraph* line_graph_;
};
