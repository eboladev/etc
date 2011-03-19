#include <QtGui>
#include <QDir>
#include <QFile>
#include <QDebug>

#include "item.h"
#include "model.h"

Model::Model(const QString &root, QObject *parent): QAbstractItemModel(parent)
{
    qDebug() << "Model::Model" << root;
    QString rootData("Name");
    rootItem = new Item(rootData);
    setupModelData(root, rootItem);
}

Model::~Model()
{
    delete rootItem;
}

int Model::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 1;
    }
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Item *item = static_cast<Item*>(index.internalPointer());
    return item->data(index.column());
}

QImage Model::image(const QModelIndex &index) const
{
    if (!index.isValid())
        return QImage();
    Item *item = static_cast<Item *>(index.internalPointer());
    return item->image();
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent)
        const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Item *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Item*>(parent.internalPointer());

    Item *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Item *childItem = static_cast<Item*>(index.internalPointer());
    Item *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int Model::rowCount(const QModelIndex &parent) const
{
    Item *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Item*>(parent.internalPointer());

    return parentItem->childCount();
}

void Model::setupModelData(const QString &root, Item *parent)
{
    QDir rootDir(root);
    QFileInfoList entries = rootDir.entryInfoList();

    foreach (QFileInfo entry, entries) {
        QString absName = entry.absoluteFilePath();
        QString relName = entry.fileName();
        if (relName.endsWith(".") || relName.endsWith("..")) {
            continue;
        }
        Item *item = new Item(absName, parent);
        parent->appendChild(item);
    }
}
