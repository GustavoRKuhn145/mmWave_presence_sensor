#pragma once

#include <Arduino.h>
#include "ui.h"

enum class IconType : uint8_t
{
    ENERGY,
    PERSON
};

const uint8_t* getButtonBitmap(
    RelayMode buttonMode,
    RelayMode activeMode,
    HomeSelection selection,
    bool hoverBlink);

const uint8_t* getIconBitmap(
    IconType icon,
    bool state);