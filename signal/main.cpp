#include <QtCore/QCoreApplication>
#include <QObject>
#include <QMetaType>

#include "client.h"
#include "gesture.h"
#include "turngesture.h"
#include "recognizer.h"
#include "turnrecognizer.h"

// This simulates a simple gesture framework with one client and
// one recognizer.

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    int ret;
    qRegisterMetaType<Gesture>("Gesture");
    qRegisterMetaType<TurnGesture>("TurnGesture");

    Client *c = new Client();
    Recognizer *r = new Recognizer();
    TurnRecognizer *t = new TurnRecognizer();

    QObject::connect(r, SIGNAL(recognized(const QVariant &)),
        c, SLOT(onRecognized(const QVariant &)),
        Qt::QueuedConnection);
    QObject::connect(t, SIGNAL(recognized(const QVariant &)),
        c, SLOT(onRecognized(const QVariant &)),
        Qt::QueuedConnection);

    r->start();
    t->start();

    ret = a.exec();

    delete t;
    delete r;
    delete c;
    return ret;
}
