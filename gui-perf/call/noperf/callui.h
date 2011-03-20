#ifndef CALLUI_H
#define CALLUI_H

#include <QLabel>

#include "phonebook.h"

class CallUi : public QWidget
{
    Q_OBJECT
public:
    explicit CallUi(QWidget *parent = 0);

signals:

public slots:
    void callComing();
    void translateNumber();

private:
    PhoneBook *phoneBook;
    QLabel *animation;
    QLabel *line1;
    QLabel *line2;
};

#endif // CALLUI_H
