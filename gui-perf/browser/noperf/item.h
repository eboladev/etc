#ifndef ITEM_H
#define ITEM_H

#include <QList>
#include <QVariant>
#include <QImage>

class Item
{
public:
    explicit Item(const QString &path, Item *parent = 0);
    ~Item();
    void appendChild(Item *child);

    Item *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    Item *parent();
    QImage image() const;

private:
    QList<Item*> childItems;
    QString itemData;
    Item *parentItem;
};

#endif // ITEM_H
