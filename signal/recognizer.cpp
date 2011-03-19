#include <QTimerEvent>
#include <QDebug>

#include "recognizer.h"

Recognizer::Recognizer(QObject *parent): QObject(parent), running(false)
{
    startTimer(1000);
}

void Recognizer::start() {
    running = true;
}

void Recognizer::stop() {
    running = false;
}

void Recognizer::timerEvent(QTimerEvent *)
{
    if (running) {
        qDebug() << "Recognizer::timerEvent";
        Gesture *g = new Gesture();
        emit recognized(g);
        delete g;
    }
}
