#include <QtGui>
#include <QThread>

#include "callui.h"
#include "phonebook.h"

CallUi::CallUi(QWidget *parent): QWidget(parent)
{
    phoneBook = new PhoneBook();
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

    connect(phoneBook, SIGNAL(numberMatched(QString)),
            this, SLOT(onNumberMatched(QString)));
    phoneBook->moveToThread(&phoneBookThread);
    phoneBookThread.start();
    phoneBookThread.setPriority(QThread::LowestPriority);

    QTimer::singleShot(3000, this, SLOT(callComing()));
}

CallUi::~CallUi()
{
    phoneBookThread.quit();
    phoneBookThread.wait();
    delete phoneBook;
}

void CallUi::callComing()
{
    line1->setText("Incoming call:");
    line2->setText("562-756-2233");

    // Request number matching from phone book
    QMetaObject::invokeMethod(phoneBook,
                              "matchNumber",
                              Q_ARG(QString, "562-756-2233"));
}

void CallUi::onNumberMatched(QString name)
{
    line2->setText(name);
}
