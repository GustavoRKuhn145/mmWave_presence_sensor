#include "ui_assets.h"
#include "bitmaps.h"

const uint8_t* getButtonBitmap(
    RelayMode buttonMode,
    RelayMode activeMode,
    HomeSelection selection,
    bool hoverBlink)
{
    bool on = false;

    // Botão confirmado
    if (buttonMode == activeMode)
    {
        on = true;
    }

    // Hover (pisca apenas se não estiver confirmado)
    else
    {
        switch(buttonMode)
        {
            case RelayMode::ON:
                if(selection == HomeSelection::ON)
                    on = hoverBlink;
                break;

            case RelayMode::AUTO:
                if(selection == HomeSelection::AUTO)
                    on = hoverBlink;
                break;

            case RelayMode::OFF:
                if(selection == HomeSelection::OFF)
                    on = hoverBlink;
                break;
        }
    }

    switch(buttonMode)
    {
        case RelayMode::ON:
            return on ? on_button_on : on_button_off;

        case RelayMode::AUTO:
            return on ? auto_button_on : auto_button_off;

        case RelayMode::OFF:
            return on ? off_button_on : off_button_off;
    }

    return on_button_off;
}

const uint8_t* getIconBitmap(
    IconType icon,
    bool state)
{
    switch(icon)
    {
        case IconType::ENERGY:
            return state ? energy_icon_on
                         : energy_icon_off;

        case IconType::PERSON:
            return state ? person_icon_on
                         : person_icon_off;
    }

    return nullptr;
}
