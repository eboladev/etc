#ifndef DELEGATE_H
#define DELEGATE_H

#include <QtGui>

class Delegate: public QItemDelegate
{
    Q_OBJECT

public:
    Delegate(QObject *parent = 0): QItemDelegate(parent) {
    }
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
        return QSize(800, 200);
    }
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

#endif // DELEGATE_H
