#include "presence.h"

#ifndef PRESENCE_PIN
#define PRESENCE_PIN 10
#endif

#ifndef PRESENCE_ACTIVE_LEVEL
#define PRESENCE_ACTIVE_LEVEL HIGH
#endif

#ifndef PRESENCE_INPUT_MODE
#define PRESENCE_INPUT_MODE INPUT_PULLDOWN
#endif

static volatile bool isrPresenceState = false;
static volatile bool isrPresenceChanged = false;

PresenceSensor presence;

static void IRAM_ATTR handlePresenceChange()
{
    isrPresenceState = digitalRead(PRESENCE_PIN) == PRESENCE_ACTIVE_LEVEL;
    isrPresenceChanged = true;
}

void PresenceSensor::begin()
{
    pinMode(PRESENCE_PIN, PRESENCE_INPUT_MODE);

    presenceState = digitalRead(PRESENCE_PIN) == PRESENCE_ACTIVE_LEVEL;
    previousPresenceState = presenceState;

    noInterrupts();
    isrPresenceState = presenceState;
    isrPresenceChanged = false;
    interrupts();

    uint32_t now = millis();
    lastPresenceMs = presenceState ? now : 0;
    countdown = 0;
    stateChanged = true;

    attachInterrupt(
        digitalPinToInterrupt(PRESENCE_PIN),
        handlePresenceChange,
        CHANGE);
}

void PresenceSensor::update(uint16_t timeoutSeconds)
{
    previousPresenceState = presenceState;
    stateChanged = false;

    loadInterruptState();

    uint32_t now = millis();

    if (presenceState)
    {
        lastPresenceMs = now;
        countdown = timeoutSeconds;
    }
    else if (lastPresenceMs > 0)
    {
        uint32_t elapsedSeconds = (now - lastPresenceMs) / 1000UL;
        countdown =
            elapsedSeconds < timeoutSeconds
                ? timeoutSeconds - elapsedSeconds
                : 0;
    }
    else
    {
        countdown = 0;
    }

    stateChanged = presenceState != previousPresenceState;
}

bool PresenceSensor::detected() const
{
    return presenceState;
}

bool PresenceSensor::changed() const
{
    return stateChanged;
}

bool PresenceSensor::consumeChanged()
{
    bool wasChanged = stateChanged;
    stateChanged = false;
    return wasChanged;
}

uint32_t PresenceSensor::countdownSeconds() const
{
    return countdown;
}

void PresenceSensor::syncUI(UIContext &context) const
{
    context.previousPresenceState = context.presenceState;
    context.presenceState = presenceState;

    context.previousPresenceCountdownSeconds =
        context.presenceCountdownSeconds;
    context.presenceCountdownSeconds = countdown;

    if (changed() ||
        context.presenceCountdownSeconds !=
            context.previousPresenceCountdownSeconds)
    {
        context.forceRedraw = true;
    }
}

void PresenceSensor::loadInterruptState()
{
    bool latestState;
    bool latestChanged;

    noInterrupts();
    latestState = isrPresenceState;
    latestChanged = isrPresenceChanged;
    isrPresenceChanged = false;
    interrupts();

    if (latestChanged)
        presenceState = latestState;
}
