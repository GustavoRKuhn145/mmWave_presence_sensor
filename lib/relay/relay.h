#pragma once

#include <Arduino.h>
#include "ui.h"

class Relay
{
public:

    void begin();

    void update(RelayMode mode, uint16_t timeoutSeconds);

    void update(
        RelayMode mode,
        uint16_t timeoutSeconds,
        bool presenceDetected);

    void on();

    void off();

    void set(bool enabled);

    bool isOn() const;

    bool presenceDetected() const;

    bool changed() const;

    uint32_t activeSeconds() const;

    void syncUI(UIContext &context) const;

private:

    void readPresence();

    void updateState(
        RelayMode mode,
        uint16_t timeoutSeconds,
        bool presenceDetected);

    void writeRelay(bool enabled);

    bool relayState = false;
    bool previousRelayState = false;

    bool presenceState = false;
    bool previousPresenceState = false;

    bool stateChanged = false;

    uint32_t lastPresenceMs = 0;
    uint32_t relayOnStartMs = 0;
    uint32_t relayActiveSeconds = 0;
};

extern Relay relay;
