#pragma once

#include <Arduino.h>
#include "ui.h"

class Encoder
{
public:

    void begin();

    void update();

    bool available() const;

    UIEvent getEvent();

private:

    bool clkState = HIGH;
    bool lastClkState = HIGH;

    bool swState = HIGH;
    bool lastSwState = HIGH;

    uint32_t pressStart = 0;

    bool longPressSent = false;

    uint32_t lastDebounce = 0;

    UIEvent pendingEvent = UIEvent::NONE;
};

extern Encoder encoder;