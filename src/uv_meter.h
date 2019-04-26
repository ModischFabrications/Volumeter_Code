#pragma once

#define FASTLED_INTERNAL // disable pragma message
#include <FastLED.h>

#include "settings.h"
#include "persistence.h"

/**
 * delayed persistence, mode is written to storage after "delay_to_save".
 * display is driven by input-events, no internal timer usage.
 * 
 * */
template <uint8_t PIN_LEDS, uint16_t N_LEDS>
class UV_Meter
{
  private:
    User_Settings settings;

    CRGB leds[N_LEDS];
    const uint8_t n_threshold_yellow;
    const uint8_t n_threshold_red;

    // "delayed persistence"
    const uint16_t delay_to_save_ms; // max 1min to prevent dataloss
    typedef unsigned long TIMESTAMP;
    TIMESTAMP next_checkpoint = 0;

    /**
     * blink a bit to show power is connected
     * (current implementation scrolls bar)
     * */
    void hello_power(uint16_t t_animation_ms)
    {
        for (uint8_t i = 0; i < N_LEDS; i++)
        {
            leds[i] = CRGB::White;
            FastLED.show();
            delay(t_animation_ms / N_LEDS); // 1 second for whole bar
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
        this->next_checkpoint = (millis() + delay_to_save_ms);

        // reset for new run
        this->settings = new_settings;
    }

    /**
     * save to EEPROM if timer has passed
     * 
     * TODO: roll-over protection
     * */
    void try_save_checkpoint()
    {
        if (this->next_checkpoint != 0 && millis() >= this->next_checkpoint)
        {
            save_settings(this->settings);
            this->next_checkpoint = 0;
        }
    }

    /**
     * write to LED, depends on internal state
     * 
     * TODO: define colors and levels as parameters
     * TODO: gradients?
     * 
     * */
    void display_level(uint8_t input_level)
    {
        // TODO: overflow?
        uint8_t n_on = (N_LEDS * input_level) / UINT8_MAX;

        for (uint8_t i = 0; i < N_LEDS; i++)
        {
            if (i > n_on) {
                leds[i] = CRGB::Black;  // OFF
            }
            else if (i > n_threshold_red)
            {
                leds[i] = CRGB::Red;
            }
            else if (i < n_threshold_yellow)
            {
                leds[i] = CRGB::Yellow;
            }
            else
            {
                leds[i] = CRGB::Green;
            }
        }
        FastLED.show();
    }

  public:
    /**
     * construct everything thats's okay "outside of time"
     * 
     * */
    UV_Meter(const uint8_t n_threshold_yellow,
             const uint8_t n_threshold_red,
             const uint16_t delay_to_save_ms) : n_threshold_yellow(n_threshold_yellow),
                                                n_threshold_red(n_threshold_red),
                                                delay_to_save_ms(delay_to_save_ms)
    {
        pinMode(PIN_LEDS, OUTPUT);

        FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(this->leds, N_LEDS);
    }

    /**
     * Call to start when everything is ready
     * 
     * loads settings from persistent storage
     * */
    void startup()
    {
        // show first to decouple from settings, 2 seconds
        hello_power(2 * 1000);

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

        /* -- test if mode is actually missed, better usability
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
        */

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
    void read(uint8_t level)
    {
        // no old levels needed
        display_level(level);

        // manage timings
        try_save_checkpoint();
    }
};
