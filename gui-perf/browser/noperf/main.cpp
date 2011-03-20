#include <QtGui>
#include <QDir>

#include "model.h"
#include "delegate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Model model(QDir::homePath() + "/Dropbox/Documents/pb/gui-perf");

    QListView view;
    view.setModel(&model);
    view.setItemDelegate(new Delegate());
    view.setWindowTitle(QObject::tr("Browser (Unoptimized)"));
    view.setGeometry(50, 50, 640, 480);
    view.show();
    return app.exec();
}
