#ifndef CALLUI_H
#define CALLUI_H

#include <QLabel>

class CallUi : public QLabel
{
    Q_OBJECT
public:
    explicit CallUi(QWidget *parent = 0);

signals:

public slots:

protected:
    void timerEvent(QTimerEvent *e);

private:
    int timerId;
    int frame;
};

#endif // CALLUI_H
