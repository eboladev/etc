#include <QStringList>
#include <QDebug>

#include "item.h"

Item::Item(const QString &path, Item *parent)
{
    parentItem = parent;
    itemData = path;
}

Item::~Item()
{
    qDeleteAll(childItems);
}

void Item::appendChild(Item *item)
{
    childItems.append(item);
}

Item *Item::child(int row)
{
    return childItems.value(row);
}

int Item::childCount() const
{
    return childItems.count();
}

int Item::columnCount() const
{
    return 1;
}

QVariant Item::data(int column) const
{
    (void)column;
    return itemData;
}

Item *Item::parent()
{
    return parentItem;
}

int Item::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<Item*>(this));

     return 0;
}

QImage Item::image() const
{
    return QImage(data(0).toString());
}
