#include <QtGui>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QCache>

#include "model.h"
#include "loader.h"

const int BATCH = 10;
static QCache<int, QImage> cache(2 * BATCH);
static QList<QModelIndex> requests;

Model::Model(const QString &root, QObject *parent): QStringListModel(parent)
{
    setupModelData(root);
    loader = new Loader(parent);
    connect(loader, SIGNAL(loadedImage(int, QImage *)),
            this, SLOT(onImageLoaded(int, QImage *)));
    loader->moveToThread(&loaderThread);
    loaderThread.start();
    loaderThread.setPriority(QThread::LowestPriority);
    timerId = startTimer(500);
}

Model::~Model()
{
    loaderThread.quit();
    loaderThread.wait();}

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

    // Otherwise, clear outstanding requests, and add request for current row,
    // and some more around it
    requests.clear();
    for (int j = 0; j < BATCH; j++) {
        requests.append(index(row - BATCH / 2 + j));
    }
    return QImage();
}

void Model::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId) {
        return;
    }

    foreach (QModelIndex i, requests) {
        requestImage(i);
    }
    requests.clear();
}

void Model::requestImage(const QModelIndex &i) const
{
    if (!i.isValid()) {
        return;
    }
    int row = i.row();
    if (!cache.contains(row)) {
        qDebug() << "Model::requestImage" << row;

        // Insert temporary image into cache
        cache.insert(row, new QImage());

        // Request image asynchronously from loader service
        QString path(data(i, Qt::DisplayRole).toString());
        QMetaObject::invokeMethod(loader,
                                  "requestImage",
                                  Q_ARG(int, row),
                                  Q_ARG(QString, path));

    }
}

void Model::onImageLoaded(int row, QImage *image)
{
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
