#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QObject>
#include <QVariant>

#include "gesture.h"

class Recognizer: public QObject
{
    Q_OBJECT

public:
    explicit Recognizer(QObject *parent = 0);
    void start();
    void stop();

signals:
    void recognized(Gesture *g);

protected:
    void timerEvent(QTimerEvent *event);
    bool running;
};

#endif // RECOGNIZER_H
