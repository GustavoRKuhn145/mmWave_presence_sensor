#include "encoder.h"

#ifndef ENCODER_CLK
#define ENCODER_CLK 2
#endif

#ifndef ENCODER_DT
#define ENCODER_DT 3
#endif

#ifndef ENCODER_SW
#define ENCODER_SW 4
#endif

constexpr uint16_t DEBOUNCE_MS = 20;

Encoder encoder;

void Encoder::begin()
{
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);

    lastClkState = digitalRead(ENCODER_CLK);
    clkState = lastClkState;

    lastSwState = digitalRead(ENCODER_SW);
    swState = lastSwState;

    pendingEvent = UIEvent::NONE;
}

bool Encoder::available() const
{
    return pendingEvent != UIEvent::NONE;
}

UIEvent Encoder::getEvent()
{
    UIEvent event = pendingEvent;
    pendingEvent = UIEvent::NONE;
    return event;
}

void Encoder::update()
{
    uint32_t now = millis();

    /* ---------------------------------------------------------- */
    /* Rotary                                                     */
    /* ---------------------------------------------------------- */

    clkState = digitalRead(ENCODER_CLK);

    if (clkState != lastClkState)
    {
        if (clkState == LOW)
        {
            bool dt = digitalRead(ENCODER_DT);

            if (dt != clkState)
                pendingEvent = UIEvent::ENCODER_CW;
            else
                pendingEvent = UIEvent::ENCODER_CCW;
        }

        lastClkState = clkState;
    }

    /* ---------------------------------------------------------- */
    /* Button                                                     */
    /* ---------------------------------------------------------- */

    swState = digitalRead(ENCODER_SW);

    if (swState != lastSwState)
    {
        if ((now - lastDebounce) < DEBOUNCE_MS)
            return;

        lastDebounce = now;
        lastSwState = swState;

        if (swState == LOW)
        {
            pressStart = now;
            longPressSent = false;
        }
        else
        {
            if (!longPressSent)
                pendingEvent = UIEvent::CLICK;
        }
    }

    if (swState == LOW)
    {
        if (!longPressSent &&
            (now - pressStart >= UI_LONG_PRESS_MS))
        {
            longPressSent = true;
            pendingEvent = UIEvent::PRESS;
        }
    }
}