#include <QDebug>

#include "turnrecognizer.h"
#include "turngesture.h"

TurnRecognizer::TurnRecognizer(QObject *parent):
    Recognizer(parent)
{
    startTimer(2000);
}

void TurnRecognizer::timerEvent(QTimerEvent *)
{
    if (running) {
        qDebug() << "TurnRecognizer::timerEvent";
        TurnGesture *g = new TurnGesture();
        delete g;
    }
}
