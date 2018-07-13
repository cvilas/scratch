#include "wrapper.h"

Wrapper::Wrapper(QObject *parent) : QObject(parent)
{
  for(int i = 0; i < 4; ++i)
  {
    auto object = new CustomObject(std::string("object_") + std::to_string(i), i);
    auto view = new CustomObjectView();
    view->setName(QString::fromStdString(object->getName()));
    view->setValue(object->getValue());
    objects_.push_back( object );
    views_.append(view);
  }

  QObject::connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimeout()));
  timer_.setSingleShot(false);
  timer_.start(1000);
}

Wrapper::~Wrapper()
{
  qDebug() << "~Wrapper";

  // Are the destructors called for elements in objects_?
}

void Wrapper::onTimeout()
{
  qDebug() << "timeout";
  static int i = 0;
  views_[i%4]->setValue(i);
  ++i;
}

//---------------------------------------------------------------------------------------------------------------------
QQmlListProperty<CustomObjectView> Wrapper::objects()
//---------------------------------------------------------------------------------------------------------------------
{
  qDebug() << "Wrapper::objects()";
  return { this, this, nullptr, &Wrapper::count, &Wrapper::getObject, nullptr };
}

//---------------------------------------------------------------------------------------------------------------------
CustomObjectView* Wrapper::getObject(int index) const
//---------------------------------------------------------------------------------------------------------------------
{
  return views_.at(index);
}

//---------------------------------------------------------------------------------------------------------------------
int Wrapper::count() const
//---------------------------------------------------------------------------------------------------------------------
{
  return views_.count();
}

//---------------------------------------------------------------------------------------------------------------------
CustomObjectView* Wrapper::getObject(QQmlListProperty<CustomObjectView>* list, int i)
//---------------------------------------------------------------------------------------------------------------------
{
  qDebug() << "Wrapper::getObject(list*, i)";
  auto obj = reinterpret_cast<Wrapper*>(list->data)->getObject(i);  // NOLINT
  qDebug() << obj->name();
  return obj;
}

//---------------------------------------------------------------------------------------------------------------------
int Wrapper::count(QQmlListProperty<CustomObjectView>* list)
//---------------------------------------------------------------------------------------------------------------------
{
  qDebug() << "Wrapper::count(list*)";
  return reinterpret_cast<Wrapper*>(list->data)->count();  // NOLINT
}
