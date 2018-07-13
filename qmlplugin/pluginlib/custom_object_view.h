#ifndef CUSTOM_OBJECT_VIEW_H
#define CUSTOM_OBJECT_VIEW_H

#include <QQuickPaintedItem>

class CustomObjectView : public QQuickPaintedItem
{
  Q_OBJECT
  Q_DISABLE_COPY(CustomObjectView)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

public:
  CustomObjectView(QQuickItem *parent = nullptr);
  ~CustomObjectView();
  QString name() const;
  void setName(const QString& name);

  int value() const;
  void setValue(int value);

signals:
  void nameChanged(QString name);
  void valueChanged(int value);

public slots:
  void incrementCount();

private:
  void paint(QPainter* painter) final;

private:
  QString name_;
  int value_;
};

#endif // CUSTOM_OBJECT_VIEW_H
