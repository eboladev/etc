#include <QtGui>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QCache>

#include "model.h"
#include "loader.h"

const int WINDOW = 20;
static QCache<int, QImage> cache(2 * WINDOW);
static QList<QModelIndex> requests;

Model::Model(const QString &root, QObject *parent): QStringListModel(parent)
{
    setupModelData(root);
    loader = new Loader(parent);
    connect(loader, SIGNAL(loadedImage(int, QImage *)),
            this, SLOT(onImageLoaded(int, QImage *)));
    timerId = startTimer(500);
    loader->moveToThread(&loaderThread);
    loaderThread.start();
    loaderThread.setPriority(QThread::LowestPriority);
}

Model::~Model()
{
    loaderThread.quit();
    loaderThread.wait();}

QVariant Model::data(const QModelIndex &i, int role) const
{
    if (role != Model::ImageRole) {
        return QStringListModel::data(i, role);
    }
    if (!i.isValid()) {
        return QVariant();
    }

    int row = i.row();
    if (cache.contains(row)) {
        return *cache[row];
    }
    requests.clear();
    requests.append(i);
    for (int j = 0; j < WINDOW; j++) {
        requests.append(index(row - WINDOW / 2 + j));
    }
    return *cache[row];
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
