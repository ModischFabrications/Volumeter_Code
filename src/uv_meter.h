#pragma once

#include <FastLED.h>

#include "settings.h"
#include "persistence.h"

/**
 * delayed persistence, mode is written after "delay_to_save"
 * 
 * display is driven by input-events, no internal timer usage
 * 
 * 
 * */
template <uint8_t PIN_LEDS, uint16_t N_LEDS> class UV_Meter
{
  private:
    User_Settings settings;

    CRGB leds[N_LEDS];

    // "delayed persistence"
    const uint16_t delay_to_save; // max 1min to prevent dataloss
    typedef unsigned long TIMESTAMP;
    TIMESTAMP next_checkpoint = 0;

    typedef uint16_t LEVEL;
    LEVEL input_level;

    /**
     * blink a bit to show power is connected
     * (current implementation scrolls bar)
     * */
    void hello_power()
    {
        for (uint8_t i = 0; i < N_LEDS; i++)
        {
            leds[i] = CRGB::White;
            FastLED.show();
            delay(1000 / N_LEDS); // 1 second for whole bar
            leds[i] = CRGB::Black;
        }
    }

    /**
     * update brightness, change current mode,
     * update persistence timer
     * */
    void apply_settings(User_Settings &new_settings)
    {
        // no need to change anything
        if (this->settings == new_settings)
            return;

        if (new_settings.brightness != this->settings.brightness)
        {
            FastLED.setBrightness(settings.brightness);
        }

        // most changes are done at runtime depending on saved settings
        if (new_settings.mode != this->settings.mode)
        {
            // TODO is there anything to do here? Maybe turn off LEDs?
        }

        // set "moving" timer to save as soon as user is done
        this->next_checkpoint = (millis() + delay_to_save);

        // reset for new run
        this->settings = new_settings;
    }

    /**
     * save to EEPROM if timer has passed
     * 
     * TODO: roll-over protection
     * */
    void try_save_checkpoint(){
        // time to save to EEPROM?
        if (this->next_checkpoint != 0 && millis() >= this->next_checkpoint)
        {
            save_settings(this->settings);
            this->next_checkpoint = 0;
        }
    }

    /**
     * write to LED, depends on internal mode
     * */
    void display_level(LEVEL input_level){
        // TODO write to LEDs 
    }

  public:
    /**
     * save to call "outside of time"
     * 
     * */
    UV_Meter(const uint16_t delay_to_save) : delay_to_save(delay_to_save)
    {
        pinMode(PIN_LEDS, OUTPUT);

        FastLED.addLeds<WS2812B, PIN_LEDS, RGB>(this->leds, N_LEDS);
    }

    /**
     * Call to start when everything is ready
     * 
     * loads settings from persistent storage
     * */
    void startup()
    {
        // show first to decouple from settings
        hello_power();

        User_Settings persistent_settings = load_settings();
        apply_settings(persistent_settings);
    }

    /**
     * cycle modes
     * 
     * TODO: not DRY
     * */
    void next_mode()
    {
        User_Settings new_settings;

        // turn back on
        if (this->settings.mode == OFF)
        {
            // lowest for all
            new_settings.mode = USER_MODE::BAR;
            new_settings.brightness = USER_BRIGHTNESS::LOW_BR;
            apply_settings(new_settings);
            return;
        }

        // delta to original settings from here on
        new_settings = this->settings;

        // try switching brightness first
        if (this->settings.brightness == LOW_BR)
        {
            new_settings.brightness = USER_BRIGHTNESS::MIDDLE_BR;
            apply_settings(new_settings);
            return;
        }
        else if (this->settings.brightness == MIDDLE_BR)
        {
            new_settings.brightness = USER_BRIGHTNESS::HIGH_BR;
            apply_settings(new_settings);
            return;
        }

        // can't increase brightness, switch to next mode
        new_settings.brightness = USER_BRIGHTNESS::LOW_BR;

        if (this->settings.mode == USER_MODE::BAR){
            new_settings.mode = USER_MODE::DOT;
            apply_settings(new_settings);
            return;
        }
        else if (this->settings.mode == USER_MODE::DOT)
        {
            new_settings.mode = USER_MODE::OFF;
            apply_settings(new_settings);
            return;
        }
        
        // turn off if nothing else was right
        new_settings.mode = USER_MODE::OFF;
        apply_settings(new_settings);
        return;
    }

    /**
     * call repeatedly from main loop to animate
     * 
     * (will also manage timings in secret)
     * */
    void set_input_level(uint16_t level){
        // no old levels needed
        display_level(level);

        // manage timings
        try_save_checkpoint();
    }

};
