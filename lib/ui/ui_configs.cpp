#include "ui.h"

#include <Fonts/Org_01.h>

/* -------------------------------------------------------------------------- */
/*                          PRIVATE DRAW FUNCTIONS                            */
/* -------------------------------------------------------------------------- */

static void drawFrame()
{
    // Barra superior
    oled->drawLine(2, 11, 125, 11, SSD1306_WHITE);

    oled->setTextColor(SSD1306_WHITE);
    oled->setFont();
    oled->setCursor(44, 2);
    oled->print("CONFIGS");
}

static void drawCursor()
{
    uint8_t y = 21;

    switch (ui.configs.selectedItem)
    {
        case ConfigItem::TIMER:
            y = 21;
            break;

        case ConfigItem::DISPLAY_TIMEOUT:
            y = 34;
            break;

        case ConfigItem::DISPLAY_BRIGHTNESS:
            y = 47;
            break;
    }

    oled->fillTriangle(
        8, y,
        3, y - 2,
        3, y + 2,
        SSD1306_WHITE);
}

static void drawLabels()
{
    oled->setTextColor(SSD1306_WHITE);
    oled->setFont();

    oled->setCursor(12,18);
    oled->print("Timer");

    oled->setCursor(12,31);
    oled->print("Desl. Tela");

    oled->setCursor(12,44);
    oled->print("Brilho");
}

static void drawTimerValue()
{
    if (ui.state == UIState::EDITING &&
        ui.configs.selectedItem == ConfigItem::TIMER &&
        !ui.configs.valueVisible)
    {
        return;
    }

    char buffer[12];

    if (ui.settings.relayTimeout < 60)
        snprintf(buffer, sizeof(buffer), "%us", ui.settings.relayTimeout);
    else if (ui.settings.relayTimeout < 3600)
        snprintf(buffer, sizeof(buffer), "%umin", ui.settings.relayTimeout / 60);
    else
        snprintf(buffer, sizeof(buffer), "%uh", ui.settings.relayTimeout / 3600);

    oled->setCursor(90,18);
    oled->print(buffer);
}

static void drawDisplayTimeout()
{
    if (ui.state == UIState::EDITING &&
        ui.configs.selectedItem == ConfigItem::DISPLAY_TIMEOUT &&
        !ui.configs.valueVisible)
    {
        return;
    }

    char buffer[12];

    if (ui.settings.displayTimeout == 0)
        snprintf(buffer, sizeof(buffer), "Nunca");
    else if (ui.settings.displayTimeout < 60)
        snprintf(buffer, sizeof(buffer), "%us", ui.settings.displayTimeout);
    else
        snprintf(buffer, sizeof(buffer), "%umin", ui.settings.displayTimeout / 60);

    oled->setCursor(90,31);
    oled->print(buffer);
}

static void drawBrightness()
{
    if (ui.state == UIState::EDITING &&
        ui.configs.selectedItem == ConfigItem::DISPLAY_BRIGHTNESS &&
        !ui.configs.valueVisible)
    {
        return;
    }

    char buffer[12];

    snprintf(
        buffer,
        sizeof(buffer),
        "%u%%",
        ui.settings.displayBrightness);

    oled->setCursor(96,44);
    oled->print(buffer);
}

/* -------------------------------------------------------------------------- */
/*                              PUBLIC FUNCTION                               */
/* -------------------------------------------------------------------------- */

void uiDrawConfigs()
{
    if (!ui.forceRedraw)
        return;

    oled->clearDisplay();

    drawFrame();

    drawCursor();

    drawLabels();

    drawTimerValue();

    drawDisplayTimeout();

    drawBrightness();

    oled->display();
}
