#include <QtGui>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QCache>

#include "model.h"
#include "loader.h"

static QCache<int, QImage> cache(500);

Model::Model(const QString &root, QObject *parent): QStringListModel(parent)
{
    setupModelData(root);
    loader = new Loader(parent);
    connect(loader, SIGNAL(loadedImage(int, QImage *)),
            this, SLOT(onImageLoaded(int, QImage *)));
    loader->moveToThread(&loaderThread);
    loaderThread.start();
    loaderThread.setPriority(QThread::LowestPriority);
}

Model::~Model()
{
    loaderThread.quit();
    loaderThread.wait();}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (role != Model::ImageRole) {
        return QStringListModel::data(index, role);
    }
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    if (cache.contains(row)) {
        qDebug() << "Model::data" << row << "served from cache";
    } else {
        qDebug() << "Model::data" << row << "not in cache";

        // Insert temporary image into cache
        cache.insert(row, new QImage());

        // Request image asynchronously
        QString path(data(index, Qt::DisplayRole).toString());
        if (!QMetaObject::invokeMethod(
                loader,
                "requestImage",
                Q_ARG(int, row),
                Q_ARG(QString, path))) {
            qCritical() << "Model::data: Invoking remote loader failed";
        }
    }
    return *cache[row];
}

void Model::onImageLoaded(int row, QImage *image)
{
    qDebug() << "Model::onImageLoaded" << row;
    cache.insert(row, image);
    QModelIndex i = index(row);
    emit dataChanged(i, i);
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
