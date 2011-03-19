#ifndef MODEL_H
#define MODEL_H

#include <QAbstractItemModel>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class Item;

class Model: public QAbstractItemModel
{
    Q_OBJECT

public:
    Model(const QString &root, QObject *parent = 0);
    ~Model();

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QImage image(const QModelIndex &index) const;

private:
    void setupModelData(const QString &root, Item *parent);

    Item *rootItem;
};

#endif // MODEL_H
