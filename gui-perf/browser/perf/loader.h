#ifndef LOADER_H
#define LOADER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QThread>

class Loader : public QObject
{
    Q_OBJECT

public:
    explicit Loader(QObject *parent = 0): QObject(parent) {
    }

public slots:
    void requestImage(int row, const QString &file);

signals:
    void loadedImage(int row, QImage *image);
};

class LoaderThread: public QThread
{
public:
    void run() {
        exec();
    }
};

#endif // LOADER_H
