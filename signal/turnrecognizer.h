#ifndef TURNRECOGNIZER_H
#define TURNRECOGNIZER_H

#include "recognizer.h"

class TurnRecognizer: public Recognizer
{
    Q_OBJECT

public:
    explicit TurnRecognizer(QObject *parent = 0);

protected:
    void timerEvent(QTimerEvent *event);
};

#endif // TURNRECOGNIZER_H
