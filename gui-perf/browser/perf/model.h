#ifndef MODEL_H
#define MODEL_H

#include <QStringListModel>
#include <QModelIndex>
#include <QVariant>
#include <QImage>
#include <QList>

#include "loader.h"

class Model: public QStringListModel
{
    Q_OBJECT

public:
    enum {
        ImageRole = Qt::UserRole + 1,
    };
    Model(const QString &root, QObject *parent = 0);
    virtual ~Model();
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void onImageLoaded(int row, QImage *image);

protected:
    void timerEvent(QTimerEvent *e);

private:
    void setupModelData(const QString &root);
    void requestImage(const QModelIndex &i) const;
    Loader *loader;
    LoaderThread loaderThread;
    int timerId;
};

#endif // MODEL_H
