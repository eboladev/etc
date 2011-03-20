#include <QThread>

#include "phonebook.h"

class Sleeper: public QThread
{
public:
    static void msleep(unsigned long msecs) {QThread::msleep(msecs);}
};

QString PhoneBook::matchNumber(const QString &)
{
    Sleeper::msleep(2000); // Simulate long processing
    return QString("Hale Berry");
}
