#include <QtGui>

#include "callui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow m;
    CallUi *call = new CallUi();
    m.setCentralWidget(call);
    m.setWindowTitle(QObject::tr("Incoming Call (Optimized)"));
    m.setGeometry(50, 50, 500, 640);
    m.show();
    return app.exec();
}
