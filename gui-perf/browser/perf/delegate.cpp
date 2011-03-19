#include "delegate.h"
#include "model.h"

Delegate::Delegate(QObject *parent): QItemDelegate(parent)
{
}

void Delegate::paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(Qt::gray));
    painter->drawRect(option.rect);
    const Model *model = qobject_cast<const Model *>(index.model());
    if (model) {
        int sourceY = model->image(index).height()/2 - option.rect.height()/2;
        painter->drawImage(option.rect.x(), option.rect.y(),
                           model->image(index), 0, sourceY);
    }
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    (void)option;
    (void)index;
    return QSize(800, 200);
}
