#include <QtGui>

#include "callui.h"

CallUi::CallUi(QWidget *parent): QLabel(parent), frame(0)
{
    setText("Ring ring");
    //timerId = startTimer(50);
    setStyleSheet("background-color: black;");
    setAlignment(Qt::AlignCenter);
    QMovie *movie = new QMovie(":/emilio.gif");
    setMovie(movie);
    movie->start();
}

void CallUi::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId) {
        return;
    }
}
