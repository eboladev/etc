#include <QDebug>

#include "loader.h"

void Loader::requestImage(int row, const QString &file)
{
    QImage *image = new QImage(file);
    emit loadedImage(row, image);
}
