#include "ui.h"
#include "ui_assets.h"

#include <Fonts/Org_01.h>

/* -------------------------------------------------------------------------- */
/*                           PRIVATE DRAW FUNCTIONS                           */
/* -------------------------------------------------------------------------- */

static void drawFrame()
{
    // Área superior (ícones)
    oled->drawRect(2, 2, 124, 30, SSD1306_WHITE);

    // Área inferior (tempo)
    oled->drawRect(2, 53, 124, 9, SSD1306_WHITE);
}

static void drawStatusIcons()
{
    oled->drawBitmap(
        33,
        5,
        getIconBitmap(IconType::ENERGY, ui.relayState),
        16,
        24,
        SSD1306_WHITE);

    oled->drawBitmap(
        72,
        5,
        getIconBitmap(IconType::PERSON, ui.presenceState),
        23,
        24,
        SSD1306_WHITE);
}

static void drawButtons()
{
    oled->drawBitmap(
        2,
        33,
        getButtonBitmap(
            RelayMode::ON,
            ui.home.activeMode,
            ui.home.selection,
            ui.home.hoverBlink),
        47,
        19,
        SSD1306_WHITE);

    oled->drawBitmap(
        35,
        33,
        getButtonBitmap(
            RelayMode::AUTO,
            ui.home.activeMode,
            ui.home.selection,
            ui.home.hoverBlink),
        58,
        19,
        SSD1306_WHITE);

    oled->drawBitmap(
        79,
        33,
        getButtonBitmap(
            RelayMode::OFF,
            ui.home.activeMode,
            ui.home.selection,
            ui.home.hoverBlink),
        47,
        19,
        SSD1306_WHITE);
}

static void drawTimer()
{
    bool showCountdown =
        !ui.presenceState &&
        ui.presenceCountdownSeconds > 0;

    uint32_t total =
        showCountdown
            ? ui.presenceCountdownSeconds
            : ui.relayActiveSeconds;

    uint16_t hours = total / 3600;
    total %= 3600;

    uint8_t minutes = total / 60;
    uint8_t seconds = total % 60;

    char buffer[24];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s:%2u:%02u:%02u",
        showCountdown ? "Timer" : "Active",
        hours,
        minutes,
        seconds);

    oled->setFont(&Org_01);

    oled->setTextColor(SSD1306_WHITE);

    oled->setCursor(4, 59);

    oled->print(buffer);
}

/* -------------------------------------------------------------------------- */
/*                               PUBLIC DRAW                                  */
/* -------------------------------------------------------------------------- */

void uiDrawHome()
{
    if (!ui.forceRedraw)
        return;

    oled->clearDisplay();

    drawFrame();

    drawStatusIcons();

    drawButtons();

    drawTimer();

    oled->display();
}
