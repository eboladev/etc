#ifndef DELEGATE_H
#define DELEGATE_H

#include <QtGui>

class Delegate: public QItemDelegate
{
    Q_OBJECT

public:
    Delegate(QObject *parent = 0);
    // bool hasParent(const QModelIndex &index) const;
    // bool isLast(const QModelIndex &index) const;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
};

#endif // DELEGATE_H
