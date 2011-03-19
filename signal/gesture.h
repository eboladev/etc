#ifndef GESTURE_H
#define GESTURE_H

#include <QObject>
#include <QMetaType>
#include <QString>
#include <QVariant>

class Gesture
{
public:
    Gesture();
    Gesture(const Gesture &other): QObject(), mName(other.mName)
    {
        Q_FOREACH(const QByteArray &prop, other.dynamicPropertyNames()) {
            setProperty(prop.constData(), other.property(prop.constData()));
        }
    }
    virtual void setName(const QString &name);
    virtual QString getName() const;

private:
    QString mName;
};

Q_DECLARE_METATYPE(Gesture)

#endif // GESTURE_H
