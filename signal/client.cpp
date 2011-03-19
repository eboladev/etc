#include <QDebug>

#include "client.h"
#include "gesture.h"
#include "turngesture.h"

Client::Client(QObject *parent): QObject(parent)
{
}

void Client::onRecognized(Gesture *g)
{
    qDebug() << "Client::onRecognized: Gesture" << g->getName();
    if (g->getName() == "turn") {
        TurnGesture *turn = qobject_cast<TurnGesture *>(g);
        if (turn) {
            qDebug() << " Angle:" << turn->getAngle();
        }
        else {
            qDebug() << " Could not convert to TurnGesture";
        }
    }
}
