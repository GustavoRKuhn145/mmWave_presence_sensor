#include "ui.h"

/* -------------------------------------------------------------------------- */
/*                           GLOBAL UI VARIABLES                              */
/* -------------------------------------------------------------------------- */

UIContext ui;

Adafruit_SSD1306 *oled = nullptr;

/* -------------------------------------------------------------------------- */
/*                           PRIVATE FUNCTIONS                                */
/* -------------------------------------------------------------------------- */

static HomeSelection nextSelection(HomeSelection current)
{
    switch (current)
    {
    case HomeSelection::OFF:
        return HomeSelection::OFF;

    case HomeSelection::AUTO:
        return HomeSelection::OFF;

    case HomeSelection::ON:
        return HomeSelection::AUTO;
    }

    return HomeSelection::AUTO;
}

static HomeSelection previousSelection(HomeSelection current)
{
    switch (current)
    {
    case HomeSelection::ON:
        return HomeSelection::ON;

    case HomeSelection::AUTO:
        return HomeSelection::ON;

    case HomeSelection::OFF:
        return HomeSelection::AUTO;
    }

    return HomeSelection::AUTO;
}

static HomeSelection selectionFromRelayMode(RelayMode mode)
{
    switch (mode)
    {
        case RelayMode::ON:
            return HomeSelection::ON;

        case RelayMode::AUTO:
            return HomeSelection::AUTO;

        case RelayMode::OFF:
            return HomeSelection::OFF;
    }

    return HomeSelection::AUTO;
}

static RelayMode relayModeFromSelection(HomeSelection selection)
{
    switch (selection)
    {
        case HomeSelection::ON:
            return RelayMode::ON;

        case HomeSelection::AUTO:
            return RelayMode::AUTO;

        case HomeSelection::OFF:
            return RelayMode::OFF;
    }

    return RelayMode::AUTO;
}

static void openHome()
{
    ui.screen = UIScreen::HOME;
    ui.state = UIState::NAVIGATION;
    ui.home.selection = selectionFromRelayMode(ui.home.activeMode);
    ui.home.hoverBlink = true;
    uint32_t now = millis();
    ui.home.lastBlink = now + UI_HOVER_DELAY_MS - UI_BLINK_INTERVAL;
    ui.home.hoverPausedUntil = now + UI_HOVER_DELAY_MS;
    ui.forceRedraw = true;
}

static ConfigItem nextConfig(ConfigItem item)
{
    switch(item)
    {
        case ConfigItem::TIMER:
            return ConfigItem::DISPLAY_TIMEOUT;

        case ConfigItem::DISPLAY_TIMEOUT:
            return ConfigItem::DISPLAY_BRIGHTNESS;

        case ConfigItem::DISPLAY_BRIGHTNESS:
            return ConfigItem::DISPLAY_BRIGHTNESS;
    }

    return ConfigItem::TIMER;
}

static ConfigItem previousConfig(ConfigItem item)
{
    switch(item)
    {
        case ConfigItem::TIMER:
            return ConfigItem::TIMER;

        case ConfigItem::DISPLAY_TIMEOUT:
            return ConfigItem::TIMER;

        case ConfigItem::DISPLAY_BRIGHTNESS:
            return ConfigItem::DISPLAY_TIMEOUT;
    }

    return ConfigItem::TIMER;
}

constexpr uint16_t RELAY_TIMEOUT_OPTIONS[] = {
    30,
    60,
    120,
    300,
    600,
    900,
    1200,
    1800,
    2700,
    3600,
    7200
};

constexpr uint16_t DISPLAY_TIMEOUT_OPTIONS[] = {
    0,
    30,
    60,
    120,
    300,
    600,
    900
};

static uint8_t findRelayTimeoutIndex()
{
    for (uint8_t i = 0; i < sizeof(RELAY_TIMEOUT_OPTIONS) / sizeof(RELAY_TIMEOUT_OPTIONS[0]); i++)
    {
        if (ui.settings.relayTimeout == RELAY_TIMEOUT_OPTIONS[i])
            return i;
    }

    return 4;
}

static uint8_t findDisplayTimeoutIndex()
{
    for (uint8_t i = 0; i < sizeof(DISPLAY_TIMEOUT_OPTIONS) / sizeof(DISPLAY_TIMEOUT_OPTIONS[0]); i++)
    {
        if (ui.settings.displayTimeout == DISPLAY_TIMEOUT_OPTIONS[i])
            return i;
    }

    return 3;
}

static void changeRelayTimeout(int8_t direction)
{
    constexpr uint8_t count =
        sizeof(RELAY_TIMEOUT_OPTIONS) / sizeof(RELAY_TIMEOUT_OPTIONS[0]);

    uint8_t index = findRelayTimeoutIndex();

    if (direction > 0)
        index = index < count - 1 ? index + 1 : count - 1;
    else
        index = index > 0 ? index - 1 : 0;

    uint16_t newValue = RELAY_TIMEOUT_OPTIONS[index];

    if (ui.settings.relayTimeout != newValue)
    {
        ui.settings.relayTimeout = newValue;
        ui.settingsChanged = true;
    }
}

static void changeDisplayTimeout(int8_t direction)
{
    constexpr uint8_t count =
        sizeof(DISPLAY_TIMEOUT_OPTIONS) / sizeof(DISPLAY_TIMEOUT_OPTIONS[0]);

    uint8_t index = findDisplayTimeoutIndex();

    if (direction > 0)
        index = index < count - 1 ? index + 1 : count - 1;
    else
        index = index > 0 ? index - 1 : 0;

    uint16_t newValue = DISPLAY_TIMEOUT_OPTIONS[index];

    if (ui.settings.displayTimeout != newValue)
    {
        ui.settings.displayTimeout = newValue;
        ui.settingsChanged = true;
    }
}

static void changeBrightness(int8_t direction)
{
    uint8_t brightness = ui.settings.displayBrightness;

    if (brightness < 10)
        brightness = 10;

    if (brightness > 100)
        brightness = 100;

    brightness = (brightness / 10) * 10;

    if (direction > 0)
        brightness = brightness < 100 ? brightness + 10 : 100;
    else
        brightness = brightness > 10 ? brightness - 10 : 10;

    if (ui.settings.displayBrightness != brightness)
    {
        ui.settings.displayBrightness = brightness;
        ui.settingsChanged = true;
    }
}

static void changeSelectedConfig(int8_t direction)
{
    switch (ui.configs.selectedItem)
    {
        case ConfigItem::TIMER:
            changeRelayTimeout(direction);
            break;

        case ConfigItem::DISPLAY_TIMEOUT:
            changeDisplayTimeout(direction);
            break;

        case ConfigItem::DISPLAY_BRIGHTNESS:
            changeBrightness(direction);
            break;
    }
}

static void pauseValueBlink()
{
    ui.configs.valueVisible = true;
    ui.configs.lastBlink = millis();
    ui.configs.blinkPausedUntil = millis() + UI_BLINK_PAUSE_MS;
}

static void pauseHomeHover()
{
    ui.home.hoverBlink = true;
    uint32_t now = millis();
    ui.home.lastBlink = now + UI_HOVER_DELAY_MS - UI_BLINK_INTERVAL;
    ui.home.hoverPausedUntil = now + UI_HOVER_DELAY_MS;
}

/* -------------------------------------------------------------------------- */
/*                                PUBLIC API                                  */
/* -------------------------------------------------------------------------- */

void uiBegin(Adafruit_SSD1306 *disp)
{
    oled = disp;

    ui.screen = UIScreen::HOME;
    ui.state = UIState::NAVIGATION;
    ui.home.selection = selectionFromRelayMode(ui.home.activeMode);
    ui.home.hoverBlink = true;

    ui.forceRedraw = true;

    uint32_t now = millis();
    ui.home.lastBlink = now + UI_HOVER_DELAY_MS - UI_BLINK_INTERVAL;
    ui.home.hoverPausedUntil = now + UI_HOVER_DELAY_MS;
    ui.configs.lastBlink = millis();
}

void uiRequestRedraw()
{
    ui.forceRedraw = true;
}

void uiUpdate()
{
    uint32_t now = millis();

    // Hover animation (Home)
    if (ui.screen == UIScreen::HOME)
    {
        if ((int32_t)(now - ui.home.hoverPausedUntil) < 0)
        {
            if (!ui.home.hoverBlink)
            {
                ui.home.hoverBlink = true;
                ui.forceRedraw = true;
            }
        }
        else if (now - ui.home.lastBlink >= UI_BLINK_INTERVAL)
        {
            ui.home.lastBlink = now;
            ui.home.hoverBlink = !ui.home.hoverBlink;

            ui.forceRedraw = true;
        }
    }

    // Value blink (Configs)
    if (ui.screen == UIScreen::CONFIGS &&
        ui.state == UIState::EDITING)
    {
        if ((int32_t)(now - ui.configs.blinkPausedUntil) < 0)
        {
            if (!ui.configs.valueVisible)
            {
                ui.configs.valueVisible = true;
                ui.forceRedraw = true;
            }
        }
        else if (now - ui.configs.lastBlink >= UI_BLINK_INTERVAL)
        {
            ui.configs.lastBlink = now;
            ui.configs.valueVisible = !ui.configs.valueVisible;
            ui.forceRedraw = true;
        }
    }

    switch (ui.screen)
    {
        case UIScreen::HOME:
            uiDrawHome();
            break;

        case UIScreen::CONFIGS:
            uiDrawConfigs();
            break;
    }

    ui.forceRedraw = false;
}

void uiHandleEvent(UIEvent event)
{
    switch (ui.screen)
    {
        /* ================================================================ */
        /*                             HOME                                */
        /* ================================================================ */

        case UIScreen::HOME:

            switch (event)
            {
                case UIEvent::ENCODER_CW:

                    ui.home.selection =
                        nextSelection(ui.home.selection);
                    pauseHomeHover();

                    ui.forceRedraw = true;
                    break;

                case UIEvent::ENCODER_CCW:

                    ui.home.selection =
                        previousSelection(ui.home.selection);
                    pauseHomeHover();

                    ui.forceRedraw = true;
                    break;

                case UIEvent::CLICK:

                    ui.home.activeMode =
                        relayModeFromSelection(ui.home.selection);

                    ui.forceRedraw = true;
                    break;

                case UIEvent::PRESS:

                    ui.screen = UIScreen::CONFIGS;
                    ui.state = UIState::NAVIGATION;

                    ui.forceRedraw = true;
                    break;

                default:
                    break;
            }

            break;

        /* ================================================================ */
        /*                           CONFIGS                               */
        /* ================================================================ */

        case UIScreen::CONFIGS:

            if (ui.state == UIState::NAVIGATION)
            {
                switch(event)
                {
                    case UIEvent::ENCODER_CW:

                        ui.configs.selectedItem =
                            nextConfig(ui.configs.selectedItem);

                        ui.forceRedraw = true;
                        break;

                    case UIEvent::ENCODER_CCW:

                        ui.configs.selectedItem =
                            previousConfig(ui.configs.selectedItem);

                        ui.forceRedraw = true;
                        break;

                    case UIEvent::CLICK:

                        ui.state = UIState::EDITING;

                        ui.configs.valueVisible = true;
                        ui.configs.lastBlink = millis();
                        ui.configs.blinkPausedUntil = millis() + UI_BLINK_PAUSE_MS;

                        ui.forceRedraw = true;
                        break;

                    case UIEvent::PRESS:

                        openHome();
                        break;

                    default:
                        break;
                }
            }
            else
            {
                switch(event)
                {
                    case UIEvent::ENCODER_CW:

                        changeSelectedConfig(1);
                        pauseValueBlink();

                        ui.forceRedraw = true;
                        break;

                    case UIEvent::ENCODER_CCW:

                        changeSelectedConfig(-1);
                        pauseValueBlink();

                        ui.forceRedraw = true;
                        break;

                    case UIEvent::PRESS:

                        ui.state = UIState::NAVIGATION;

                        ui.configs.valueVisible = true;
                        ui.configs.blinkPausedUntil = 0;

                        ui.forceRedraw = true;
                        break;

                    default:
                        break;
                }
            }

            break;
    }
}
