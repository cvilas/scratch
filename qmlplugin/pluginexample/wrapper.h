#ifndef WRAPPER_H
#define WRAPPER_H

#include <QObject>
#include <QQmlListProperty>
#include <QTimer>

#include "custom_object_view.h"
#include "custom_object.h"

class Wrapper : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QQmlListProperty<CustomObjectView> objects READ objects)

public:
  explicit Wrapper(QObject *parent = nullptr);
  ~Wrapper();
  QQmlListProperty<CustomObjectView> objects();

  int count() const;
  CustomObjectView* getObject(int) const;

private slots:
  void onTimeout();

private:
  static int count(QQmlListProperty<CustomObjectView>*);
  static CustomObjectView* getObject(QQmlListProperty<CustomObjectView>*, int);

private:
  QList<CustomObjectView*> views_;
  QList<CustomObject*> objects_;
  QTimer timer_;
};

#endif // WRAPPER_H
