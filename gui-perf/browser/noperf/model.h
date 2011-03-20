#ifndef MODEL_H
#define MODEL_H

#include <QStringListModel>
#include <QModelIndex>
#include <QVariant>
#include <QImage>
#include <QList>

class Model: public QStringListModel
{
    Q_OBJECT

public:
    enum {
        ImageRole = Qt::UserRole + 1,
    };
    Model(const QString &root, QObject *parent = 0);
    virtual ~Model();
    QVariant data(const QModelIndex &index, int role) const;

private:
    void setupModelData(const QString &root);
};

#endif // MODEL_H
