#include "delegate.h"
#include "model.h"

Delegate::Delegate(QObject *parent): QItemDelegate(parent)
{
}

void Delegate::paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
    painter->setBrush(QBrush(Qt::darkGray));
    painter->setPen(QPen(Qt::gray));
    painter->drawRect(option.rect);

    const Model *model = qobject_cast<const Model *>(index.model());
    if (model) {
        QImage image = model->data(index, Model::ImageRole).value<QImage>();
        int sourceY = image.height() / 2 - option.rect.height() / 2;
        painter->drawImage(option.rect.x(), option.rect.y(),
                           image, 0, sourceY);

        QString label = model->data(index, Qt::DisplayRole).toString();
        QFont font = painter->font();
        font.setPixelSize(22);
        painter->setFont(font);
        painter->setPen(QPen(Qt::black));
        painter->drawText(option.rect.x() + 21, option.rect.y() + 181, label);
        painter->setPen(QPen(Qt::white));
        painter->drawText(option.rect.x() + 20, option.rect.y() + 180, label);
    }
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    (void)option;
    (void)index;
    return QSize(800, 200);
}
