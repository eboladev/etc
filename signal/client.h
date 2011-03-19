#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariant>

#include "gesture.h"

class Client: public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = 0);

signals:

public slots:
    void onRecognized(Gesture *g);
};

#endif // CLIENT_H
