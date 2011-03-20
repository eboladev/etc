#ifndef CALLUI_H
#define CALLUI_H

#include <QLabel>

#include "phonebook.h"

class CallUi : public QWidget
{
    Q_OBJECT

public:
    explicit CallUi(QWidget *parent = 0);
    virtual ~CallUi();

signals:

public slots:
    void callComing();
    void onNumberMatched(QString name);

private:
    PhoneBook *phoneBook;
    QLabel *animation;
    QLabel *line1;
    QLabel *line2;
    PhoneBookThread phoneBookThread;
};

#endif // CALLUI_H
