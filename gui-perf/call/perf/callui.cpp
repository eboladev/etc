#include <QtGui>

#include "callui.h"

CallUi::CallUi(QWidget *parent): QLabel(parent), frame(0)
{
    setText("Ring ring");
    timerId = startTimer(50);
    setStyleSheet("background-color: black;");
    setAlignment(Qt::AlignCenter);
}

void CallUi::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId) {
        return;
    }

    setPixmap(QPixmap(QString(":/flower-%1.png").arg(frame++)));
    if (frame > 13) {
        frame = 0;
    }
}
