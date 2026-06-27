#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr uint16_t UI_BLINK_INTERVAL = 300;
constexpr uint16_t UI_BLINK_PAUSE_MS = 700;
constexpr uint16_t UI_HOVER_DELAY_MS = 350;
constexpr uint16_t UI_LONG_PRESS_MS = 600;

enum class UIScreen: uint8_t
{
    HOME,
    CONFIGS
};

enum class UIState: uint8_t
{
    NAVIGATION,
    EDITING
};

enum class RelayMode: uint8_t
{
    ON,
    AUTO,
    OFF
};

enum class HomeSelection : uint8_t
{
    ON,
    AUTO,
    OFF
};

enum class ConfigItem : uint8_t
{
    TIMER              = 0,
    DISPLAY_TIMEOUT    = 1,
    DISPLAY_BRIGHTNESS = 2
};

enum class UIEvent: uint8_t
{
    NONE,

    ENCODER_CW,
    ENCODER_CCW,

    CLICK,
    PRESS
};

struct UISettings
{
    uint16_t relayTimeout = 600;
    uint16_t displayTimeout = 120;
    uint8_t displayBrightness = 100;
};

struct HomeState
{
    RelayMode activeMode = RelayMode::AUTO;
    HomeSelection selection = HomeSelection::AUTO;

    bool hoverBlink = false;
    uint32_t lastBlink = 0;
    uint32_t hoverPausedUntil = 0;
};

struct ConfigState
{
    ConfigItem selectedItem = ConfigItem::TIMER;

    bool valueVisible = true;
    uint32_t lastBlink = 0;
    uint32_t blinkPausedUntil = 0;
};

struct UIContext
{
    UIScreen screen = UIScreen::HOME;

    UIState state = UIState::NAVIGATION;

    HomeState home;
    ConfigState configs;

    UISettings settings;

    bool relayState = false;
    bool previousRelayState = false;

    bool presenceState = false;
    bool previousPresenceState = false;

    uint32_t relayActiveSeconds = 0;
    uint32_t previousRelayActiveSeconds = 0;

    uint32_t presenceCountdownSeconds = 0;
    uint32_t previousPresenceCountdownSeconds = 0;

    bool settingsChanged = false;
    bool forceRedraw = true;
};

extern UIContext ui;

extern Adafruit_SSD1306 *oled;

void uiBegin(Adafruit_SSD1306 *oled);

void uiUpdate();

void uiHandleEvent(UIEvent event);

void uiRequestRedraw();

void uiDrawHome();

void uiDrawConfigs();
