#pragma once
#include <QAbstractListModel>
#include <QVector>
#include "custom_object_view.h"

/// see http://doc.qt.io/qt-5/qtquick-modelviewsdata-cppmodels.html

class Model : public QAbstractListModel {
  Q_OBJECT
public:
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

public slots:
  void insert(CustomObjectView* item);
  void remove(CustomObjectView* item);

  void testAutoIncList();
  void testModifyList();

protected:
  QHash<int, QByteArray> roleNames() const override;

private:
  QVector<CustomObjectView*> mItems;
};
