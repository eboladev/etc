#include <QDebug>

#include "loader.h"

Loader::Loader(QObject *parent): QObject(parent)
{
}

void Loader::requestImage(int row, const QString &file)
{
    QImage *image = new QImage(file);
    emit loadedImage(row, image);
}
