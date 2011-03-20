#include <QtGui>

#include "callui.h"

CallUi::CallUi(QWidget *parent): QLabel(parent), frame(0)
{
    setText("Ring ring");
    timerId = startTimer(90);
}

void CallUi::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId) {
        return;
    }

    setPixmap(QPixmap(QString(":/flower-%1.png").arg(frame++)));
    if (frame > 20) {
        frame = 0;
    }
}
