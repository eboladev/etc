#include <QtGui>
#include <QThread>

#include "callui.h"
#include "phonebook.h"

CallUi::CallUi(QWidget *parent): QWidget(parent)
{
    phoneBook = new PhoneBook(this);
    animation = new QLabel(this);
    line1 = new QLabel(" ", this);
    line2 = new QLabel(" ", this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(line1);
    layout->addWidget(line2);
    layout->addWidget(animation);
    QMovie *movie = new QMovie(":/emilio.gif", QByteArray(), this);
    animation->setMovie(movie);
    movie->start();
    QTimer::singleShot(3000, this, SLOT(callComing()));
}

void CallUi::callComing()
{
    line1->setText("Incoming call:");
    line2->setText("562-756-2233");
    QString name = phoneBook->matchNumber("562-756-2233");
    line2->setText(name);
}
