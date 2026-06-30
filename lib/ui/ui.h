#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr uint16_t UI_BLINK_INTERVAL = 300;
constexpr uint16_t UI_BLINK_PAUSE_MS = 700;
constexpr uint16_t UI_HOVER_DELAY_MS = 350;
constexpr uint16_t UI_LONG_PRESS_MS = 600;

constexpr uint16_t RELAY_TIMEOUT_OPTIONS[] = {
    0,
    10,
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

// Telas
enum class UIScreen: uint8_t
{
    HOME, // Tela inicial
    CONFIGS // Tela de configurações
};

// Estados da UI
enum class UIState: uint8_t
{
    NAVIGATION, // Modo de navegação (selecionando itens)
    EDITING, // Modo de edição (alterando valores)
};

// Estrutura de dados para armazenar o estado atual do relé da UI
enum class RelayMode: uint8_t
{
    ON,
    AUTO,
    OFF
};

// Estrutura de dados para armazenar a seleção atual da tela inicial da UI
enum class HomeSelection : uint8_t
{
    ON,
    AUTO,
    OFF
};

// Itens de configuração disponíveis na tela de configurações da UI
enum class ConfigItem : uint8_t
{
    TIMER              = 0,
    DISPLAY_TIMEOUT    = 1,
    DISPLAY_BRIGHTNESS = 2
};

// Eventos do encoder e botão
enum class UIEvent: uint8_t
{
    NONE,

    ENCODER_CW,
    ENCODER_CCW,

    CLICK,
    PRESS
};

// Valores padrão da Tela de Configurações
struct UISettings
{
    uint16_t relayTimeout = 600;
    uint16_t displayTimeout = 120;
    uint8_t displayBrightness = 100;
};

// Valores padrão da Tela Inicial
struct HomeState
{
    RelayMode activeMode = RelayMode::AUTO;
    HomeSelection selection = HomeSelection::AUTO;

    bool hoverBlink = false;
    uint32_t lastBlink = 0;
    uint32_t hoverPausedUntil = 0;
};

// Usado para controlar o piscar do valor selecionado nas telas
struct ConfigState
{
    ConfigItem selectedItem = ConfigItem::TIMER;

    bool valueVisible = true;
    uint32_t lastBlink = 0;
    uint32_t blinkPausedUntil = 0;
};

// Estrutura de dados para armazenar o estado atual da UI
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
