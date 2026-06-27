#include "relay.h"

#ifndef RELAY_PIN
#define RELAY_PIN 0
#endif

#ifndef PRESENCE_PIN
#define PRESENCE_PIN 10
#endif

#ifndef RELAY_ACTIVE_LEVEL
#define RELAY_ACTIVE_LEVEL HIGH
#endif

#ifndef PRESENCE_ACTIVE_LEVEL
#define PRESENCE_ACTIVE_LEVEL HIGH
#endif

#ifndef PRESENCE_INPUT_MODE
#define PRESENCE_INPUT_MODE INPUT_PULLDOWN
#endif

Relay relay;

void Relay::begin()
{
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(PRESENCE_PIN, PRESENCE_INPUT_MODE);

    previousRelayState = false;
    relayState = false;
    writeRelay(false);

    previousPresenceState = digitalRead(PRESENCE_PIN) == PRESENCE_ACTIVE_LEVEL;
    presenceState = previousPresenceState;

    uint32_t now = millis();
    lastPresenceMs = presenceState ? now : 0;
    relayOnStartMs = 0;
    relayActiveSeconds = 0;
    stateChanged = true;
}

void Relay::update(RelayMode mode, uint16_t timeoutSeconds)
{
    previousRelayState = relayState;
    previousPresenceState = presenceState;
    stateChanged = false;

    readPresence();

    updateState(mode, timeoutSeconds, presenceState);
}

void Relay::update(
    RelayMode mode,
    uint16_t timeoutSeconds,
    bool detected)
{
    previousRelayState = relayState;
    previousPresenceState = presenceState;
    stateChanged = false;

    presenceState = detected;

    updateState(mode, timeoutSeconds, presenceState);
}

void Relay::updateState(
    RelayMode mode,
    uint16_t timeoutSeconds,
    bool detected)
{
    uint32_t now = millis();

    if (detected)
        lastPresenceMs = now;

    bool shouldEnable = false;

    switch (mode)
    {
        case RelayMode::ON:
            shouldEnable = true;
            break;

        case RelayMode::AUTO:
            shouldEnable =
                detected ||
                (lastPresenceMs > 0 &&
                 (now - lastPresenceMs) < (uint32_t)timeoutSeconds * 1000UL);
            break;

        case RelayMode::OFF:
            shouldEnable = false;
            break;
    }

    writeRelay(shouldEnable);

    if (relayState)
    {
        if (!previousRelayState)
            relayOnStartMs = now;

        relayActiveSeconds = (now - relayOnStartMs) / 1000UL;
    }
    else
    {
        relayOnStartMs = 0;
        relayActiveSeconds = 0;
    }

    stateChanged =
        relayState != previousRelayState ||
        presenceState != previousPresenceState;
}

void Relay::on()
{
    writeRelay(true);
}

void Relay::off()
{
    writeRelay(false);
}

void Relay::set(bool enabled)
{
    writeRelay(enabled);
}

bool Relay::isOn() const
{
    return relayState;
}

bool Relay::presenceDetected() const
{
    return presenceState;
}

bool Relay::changed() const
{
    return stateChanged;
}

uint32_t Relay::activeSeconds() const
{
    return relayActiveSeconds;
}

void Relay::syncUI(UIContext &context) const
{
    context.previousRelayState = context.relayState;
    context.relayState = relayState;

    context.previousPresenceState = context.presenceState;
    context.presenceState = presenceState;

    context.previousRelayActiveSeconds = context.relayActiveSeconds;
    context.relayActiveSeconds = relayActiveSeconds;

    if (changed() ||
        context.relayActiveSeconds != context.previousRelayActiveSeconds)
    {
        context.forceRedraw = true;
    }
}

void Relay::readPresence()
{
    presenceState = digitalRead(PRESENCE_PIN) == PRESENCE_ACTIVE_LEVEL;
}

void Relay::writeRelay(bool enabled)
{
    relayState = enabled;
    digitalWrite(
        RELAY_PIN,
        enabled ? RELAY_ACTIVE_LEVEL : !RELAY_ACTIVE_LEVEL);
}
