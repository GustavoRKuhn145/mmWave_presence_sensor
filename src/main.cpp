#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#include "encoder.h"
#include "presence.h"
#include "relay.h"
#include "ui.h"

#ifndef OLED_WIDTH
#define OLED_WIDTH 128
#endif

#ifndef OLED_HEIGHT
#define OLED_HEIGHT 64
#endif

#ifndef OLED_RESET
#define OLED_RESET -1
#endif

#ifndef OLED_ADDRESS
#define OLED_ADDRESS 0x3C
#endif

#ifndef OLED_SDA
#define OLED_SDA 8
#endif

#ifndef OLED_SCL
#define OLED_SCL 9
#endif

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
Preferences preferences;

static bool displayEnabled = true;
static bool displayStarted = false;
static uint32_t lastUserActivityMs = 0;
static uint8_t lastBrightness = 255;
static UISettings savedSettings;
static bool settingsSavePending = false;
static uint32_t settingsSaveAtMs = 0;


template <typename T, size_t N>
static bool containsOption(const T (&options)[N], T value)
{
    for (size_t i = 0; i < N; i++)
    {
        if (options[i] == value)
            return true;
    }

    return false;
}

static void validateSettings()
{
    if (!containsOption(RELAY_TIMEOUT_OPTIONS, ui.settings.relayTimeout))
        ui.settings.relayTimeout = 600;

    if (!containsOption(DISPLAY_TIMEOUT_OPTIONS, ui.settings.displayTimeout))
        ui.settings.displayTimeout = 120;

    if (ui.settings.displayBrightness < 10 ||
        ui.settings.displayBrightness > 100)
    {
        ui.settings.displayBrightness = 100;
    }

    ui.settings.displayBrightness =
        (ui.settings.displayBrightness / 10) * 10;
}

static void loadSettings()
{
    preferences.begin("mmwave", true);

    ui.settings.relayTimeout =
        preferences.getUShort("relay_to", ui.settings.relayTimeout);
    ui.settings.displayTimeout =
        preferences.getUShort("disp_to", ui.settings.displayTimeout);
    ui.settings.displayBrightness =
        preferences.getUChar("bright", ui.settings.displayBrightness);

    preferences.end();

    validateSettings();
    savedSettings = ui.settings;
    ui.settingsChanged = false;
}

static bool settingsDiffer(const UISettings &a, const UISettings &b)
{
    return
        a.relayTimeout != b.relayTimeout ||
        a.displayTimeout != b.displayTimeout ||
        a.displayBrightness != b.displayBrightness;
}

static void saveSettings()
{
    validateSettings();

    if (!settingsDiffer(ui.settings, savedSettings))
        return;

    preferences.begin("mmwave", false);
    preferences.putUShort("relay_to", ui.settings.relayTimeout);
    preferences.putUShort("disp_to", ui.settings.displayTimeout);
    preferences.putUChar("bright", ui.settings.displayBrightness);
    preferences.end();

    savedSettings = ui.settings;
}

static void updateSettingsPersistence()
{
    if (ui.settingsChanged)
    {
        settingsSavePending = true;
        settingsSaveAtMs = millis() + 1000UL;
        ui.settingsChanged = false;
    }

    if (!settingsSavePending)
        return;

    if ((int32_t)(millis() - settingsSaveAtMs) < 0)
        return;

    settingsSavePending = false;
    saveSettings();
}

static uint8_t brightnessToContrast(uint8_t brightness)
{
    if (brightness > 100)
        brightness = 100;

    if (brightness < 10)
        brightness = 10;

    uint16_t scaled = (uint16_t)brightness * brightness;
    return map(scaled, 100, 10000, 1, 255);
}

static uint8_t brightnessToPrecharge(uint8_t brightness)
{
    if (brightness > 100)
        brightness = 100;

    if (brightness < 10)
        brightness = 10;

    return map(brightness, 10, 100, 0x11, 0xF1);
}

static void applyDisplayBrightness()
{
    if (!displayStarted)
        return;

    if (ui.settings.displayBrightness == lastBrightness)
        return;

    lastBrightness = ui.settings.displayBrightness;

    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightnessToContrast(lastBrightness));
    display.ssd1306_command(SSD1306_SETPRECHARGE);
    display.ssd1306_command(brightnessToPrecharge(lastBrightness));
}

static void wakeDisplay()
{
    lastUserActivityMs = millis();

    if (!displayStarted || displayEnabled)
        return;

    display.ssd1306_command(SSD1306_DISPLAYON);
    displayEnabled = true;
    uiRequestRedraw();
}

static void updateDisplayPower()
{
    if (!displayStarted)
        return;

    if (!displayEnabled)
        return;

    uint32_t timeoutMs = (uint32_t)ui.settings.displayTimeout * 1000UL;

    if (timeoutMs == 0)
        return;

    if (millis() - lastUserActivityMs >= timeoutMs)
    {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        displayEnabled = false;
    }
}

static void handleEncoder()
{
    encoder.update();

    if (!encoder.available())
        return;

    UIEvent event = encoder.getEvent();

    wakeDisplay();
    uiHandleEvent(event);
}

static void updatePresenceAndRelay()
{
    presence.update(ui.settings.relayTimeout);
    presence.syncUI(ui);

    if (presence.changed())
        wakeDisplay();

    relay.update(
        ui.home.activeMode,
        ui.settings.relayTimeout,
        presence.detected());

    relay.syncUI(ui);
}

void setup()
{
    Serial.begin(115200);
    loadSettings();

    Wire.begin(OLED_SDA, OLED_SCL);

    displayStarted = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

    if (displayStarted)
    {
        display.clearDisplay();
        display.display();
        uiBegin(&display);
        applyDisplayBrightness();
    }

    encoder.begin();
    presence.begin();
    relay.begin();

    lastUserActivityMs = millis();
}

void loop()
{
    handleEncoder();
    updatePresenceAndRelay();
    applyDisplayBrightness();
    updateSettingsPersistence();

    if (displayStarted && displayEnabled)
        uiUpdate();

    updateDisplayPower();

    delay(5);
}
