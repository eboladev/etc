#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <QObject>
#include <QThread>

class PhoneBook: public QObject
{
    Q_OBJECT

public:
    explicit PhoneBook(QObject *parent = 0): QObject(parent) {}

public slots:
    void matchNumber(const QString &number);

signals:
    void numberMatched(QString name);
};

class PhoneBookThread: public QThread
{
public:
    void run() {
        exec();
    }
};

#endif // PHONEBOOK_H
