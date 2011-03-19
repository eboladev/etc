#include "delegate.h"
#include "model.h"

void Delegate::paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
    // Draw background
    painter->setBrush(QBrush(Qt::darkGray));
    painter->setPen(QPen(Qt::gray));
    painter->drawRect(option.rect);

    // Get and draw image from model
    const QAbstractItemModel *model = index.model();
    QImage image = model->data(index, Model::ImageRole).value<QImage>();
    int sourceY = image.height() / 2 - option.rect.height() / 2;
    painter->drawImage(option.rect.x(), option.rect.y(), image, 0, sourceY);

    // Draw label
    QString label = QString().setNum(index.row());
    QFont font = painter->font();
    font.setPixelSize(48);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(QPen(Qt::black));
    painter->drawText(option.rect.x() + 21, option.rect.y() + 181, label);
    painter->setPen(QPen(Qt::white));
    painter->drawText(option.rect.x() + 20, option.rect.y() + 180, label);
}
