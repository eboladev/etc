#include <QtGui>
#include <QThread>

#include "callui.h"

class Sleeper: public QThread
{
public:
    static void msleep(unsigned long msecs) {QThread::msleep(msecs);}
};

CallUi::CallUi(QWidget *parent): QLabel(parent), frame(0)
{
    setText("Ring ring");
    timerId = startTimer(1500);
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

    Sleeper::msleep(700);
}
