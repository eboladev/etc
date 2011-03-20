#include <QThread>

#include "phonebook.h"

class Sleeper: public QThread
{
public:
    static void msleep(unsigned long msecs) {QThread::msleep(msecs);}
};

void PhoneBook::matchNumber(const QString &)
{
    Sleeper::msleep(3000); // Simulate long processing
    QString name("Hale Berry");
    emit numberMatched(name);
}
