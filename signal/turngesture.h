#ifndef TURNGESTURE_H
#define TURNGESTURE_H

#include "gesture.h"

class TurnGesture: public Gesture
{
public:
    TurnGesture();
    int angle;
};

Q_DECLARE_METATYPE(TurnGesture)

#endif // TURNGESTURE_H
