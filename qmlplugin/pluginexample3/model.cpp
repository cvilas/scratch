#include "model.h"

int Model::rowCount(const QModelIndex&) const {
  return mItems.size();
}

QVariant Model::data(const QModelIndex& index, int /*role*/) const {
  CustomObjectView* item = mItems.at(index.row());
  return QVariant::fromValue(item);
}

void Model::insert(CustomObjectView* item) {
  beginInsertRows(QModelIndex(), 0, 0);
  mItems.push_back(item);
  endInsertRows();
}

void Model::remove(CustomObjectView* item) {
  for (int i = 0; i < mItems.size(); ++i) {
    if (mItems.at(i) == item) {
      beginRemoveRows(QModelIndex(), i, i);
      mItems.remove(i);
      endRemoveRows();
      break;
    }
  }
}

void Model::testModifyList()
{
  static int n = 100;

  int i = 0;
  QString name = QString("modified name ") + QString::number(n);
  int value = n;

  ++n;

  /// Test modifying an item in the list and see if QML view updates
  if((i >= 0) && (i < mItems.size()))
  {
    mItems[i]->setName(name);
    mItems[i]->setValue(value);

    /// \todo do we need this?
    emit dataChanged(this->index(i), this->index(i), {Qt::UserRole + 1});

    qDebug() << "modified " << i;
  }
}

void Model::testAutoIncList()
{
  /// Tests whether we can dynamically add new items to the model and see it on the QML view
  ///
  static int n = 10;
  auto item = new CustomObjectView;
  item->setName(QString("test object") + QString::number(n));
  item->setValue(n);
  ++n;
  insert(item);
}

QHash<int, QByteArray> Model::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Qt::UserRole + 1] = "item";
  return roles;
}
