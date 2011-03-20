#include <QtGui>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QCache>

#include "model.h"

const int BATCH = 18;
static QCache<int, QImage> cache(50);

Model::Model(const QString &root, QObject *parent): QStringListModel(parent)
{
    setupModelData(root);
}

Model::~Model()
{
}

QVariant Model::data(const QModelIndex &i, int role) const
{
    // Requesting data for invalid index
    if (!i.isValid()) {
        return QVariant();
    }

    // Requesting non-image data
    if (role != Model::ImageRole) {
        return QStringListModel::data(i, role);
    }

    // Requesting image data

    int row = i.row();

    // If image is in the cache, serve it from there
    if (cache.contains(row)) {
        return *cache[row];
    }

    // Otherwise, load image from file
    QString path(data(i, Qt::DisplayRole).toString());
    QImage *image = new QImage(path);
    cache.insert(row, image);
    return *image;
}

void Model::setupModelData(const QString &root)
{
    QStringList items;
    QDir rootDir(root);
    QFileInfoList entries = rootDir.entryInfoList();

    foreach (QFileInfo entry, entries) {
        QString absName = entry.absoluteFilePath();
        QString relName = entry.fileName();
        if (relName.endsWith(".") || relName.endsWith("..")) {
            continue;
        }
        items.append(absName);
    }

    setStringList(items);
}
