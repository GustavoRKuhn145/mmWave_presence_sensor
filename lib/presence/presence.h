#pragma once

#include <Arduino.h>
#include "ui.h"

class PresenceSensor
{
public:

    void begin();

    void update(uint16_t timeoutSeconds);

    bool detected() const;

    bool changed() const;

    bool consumeChanged();

    uint32_t countdownSeconds() const;

    void syncUI(UIContext &context) const;

private:

    void loadInterruptState();

    bool presenceState = false;
    bool previousPresenceState = false;
    bool stateChanged = false;

    uint32_t lastPresenceMs = 0;
    uint32_t countdown = 0;
};

extern PresenceSensor presence;
