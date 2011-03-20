#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <QObject>

class PhoneBook: public QObject
{
    Q_OBJECT

public:
    explicit PhoneBook(QObject *parent = 0): QObject(parent) {}
    QString matchNumber(const QString &number);

signals:

public slots:

};

#endif // PHONEBOOK_H
