#include "custom_object_view.h"

#include <QColor>
#include <QPainter>
#include <QDebug>

CustomObjectView::CustomObjectView(QQuickItem* parent) : QQuickPaintedItem(parent), name_(""), value_(0)
{
  // By default, QQuickItem does not draw anything. If you subclass
  // QQuickItem to create a visual item, you will need to uncomment the
  // following line and re-implement updatePaintNode()

  // setFlag(ItemHasContents, true);
}

CustomObjectView::~CustomObjectView()
{
  qDebug() << "[~CustomObjectView] (" << name() << ")";
}

void CustomObjectView::paint(QPainter *painter)
{
  QPen pen(Qt::red, 2);
  painter->setPen(pen);
  painter->setRenderHints(QPainter::Antialiasing, true);
  painter->drawPie(boundingRect().adjusted(1, 1, -1, -1), 90 * 16, 290 * 16);

  painter->drawText(QRectF(10, 10, 100, 20), name());
  painter->drawText(QRectF(10, 40, 100, 20), QString::number(value()));
}

QString CustomObjectView::name() const
{
  return name_;
}

void CustomObjectView::setName(const QString& name)
{
  name_ = name;
  //emit nameChanged(name);
  //update();
}

int CustomObjectView::value() const
{
  return value_;
}

void CustomObjectView::setValue(int value)
{
  value_ = value;
  emit valueChanged(value);
  update();
}

void CustomObjectView::incrementCount()
{
  setValue( value() + 1 );
}
