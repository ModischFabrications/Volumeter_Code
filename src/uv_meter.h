#pragma once

#define FASTLED_INTERNAL // disable pragma message
#include <FastLED.h>

#include "settings.h"
#include "persistence.h"

// --- constants

const bool VERBOSE = true;
const uint16_t delay_verbose_ms = (1 * 500);

const bool DUALMODE = false;

const CRGB C_OK = CRGB::White;
const CRGB C_WARN = CRGB::Yellow;
const CRGB C_CRIT = CRGB::Red;

/**
 * delayed persistence, mode is written to storage after "delay_to_save".
 * display is driven by input-events, no internal timer usage.
 * 
 * Timing is done with each reading, as there is nothing that needs 
 * a finer resolution. This makes `tick()` obsolete.
 * 
 * This implementation will try to ignore all mistakes as it
 * can't really throw any exceptions as these crash the uC...
 * 
 * */
template <uint8_t PIN_LEDS, uint16_t N_LEDS>
class UV_Meter
{
private:
    User_Settings settings;

    CRGB leds[N_LEDS];

    // can't be global constants because of N_LEDS
    const uint8_t N_THRESHOLD_WARN = (N_LEDS * 0.5);
    const uint8_t N_THRESHOLD_CRIT = (N_LEDS * 0.8);

    // "delayed persistence"
    const uint16_t delay_to_save_ms; // max 1min to prevent dataloss
    typedef unsigned long TIMESTAMP;
    TIMESTAMP t_next_savepoint = 0;

    TIMESTAMP t_lock_output_until = 0;

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
            // show changes
            FastLED.show();
        }

        // most changes are done at runtime depending on saved settings
        if (new_settings.mode != this->settings.mode)
        {
            // TODO is there anything to do here? Maybe turn off LEDs?
        }

        // set "moving" timer to save as soon as user is done
        this->t_next_savepoint = (millis() + delay_to_save_ms);

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
        if (this->t_next_savepoint != 0 && millis() >= this->t_next_savepoint)
        {
            save_settings(this->settings);
            this->t_next_savepoint = 0;
        }
    }

    /**
     * write level to LEDs according to the selected mode
     * 
     * TODO: gradients? `fill_gradient` in two segments? `blend`
     * fade last LED? prevent jumping (value is decimals after division)
     * 
     * */
    void display_level(uint8_t input_level)
    {
        // was set previously
        if (this->t_lock_output_until != 0)
        {
            if (millis() <= this->t_lock_output_until)
            {
                // overriding output is forbidden now
                return;
            }
            else // reached only once per lock
            {
                // reset to prevent locking again when overflowing
                this->t_lock_output_until = 0;
            }
        }

        // TODO: is this safe with overflowing values (> 1 day)?
        uint8_t n_on = (N_LEDS * input_level) / UINT8_MAX;

        // set color for individual leds
        // TODO: implement dot mode
        for (uint8_t i = 0; i < N_LEDS; i++)
        {
            if (i > n_on)
            {
                leds[i] = CRGB::Black; // OFF
            }
            else if (i > N_THRESHOLD_CRIT)
            {
                leds[i] = C_CRIT;
            }
            else if (i < N_THRESHOLD_WARN)
            {
                leds[i] = C_WARN;
            }
            else
            {
                leds[i] = C_OK;
            }
        }

        FastLED.show();
    }

public:
    /**
     * do everything thats's okay "outside of time"
     * 
     * */
    UV_Meter(const uint16_t delay_to_save_ms) : delay_to_save_ms(delay_to_save_ms)
    {
        pinMode(PIN_LEDS, OUTPUT);

        FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(this->leds, N_LEDS);
        // set powerlimit to 5v, 1000mA (Fuse size)
        FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
    }

    /**
     * Load settings and show readiness. 
     * 
     * loads settings from persistent storage
     * */
    void startup()
    {
        // show first to decouple from settings, 2 seconds
        hello_power(1 * 1000);

        // FIXME: this is broken
        //User_Settings persistent_settings = load_settings();
        //apply_settings(persistent_settings);

        if (VERBOSE)
        {
            flash(CRGB::White);
        }
    }

    /**
     * flash shortly to signal something.
     * Designed for debug use only.
     * 
     * */
    void flash(CRGB color, uint16_t min_duration = delay_verbose_ms)
    {
        // decrease to prevent burning out your eyes while debugging
        uint8_t prev_brightness = FastLED.getBrightness();
        FastLED.setBrightness(32);

        fill_solid(this->leds, N_LEDS, color);
        FastLED.show();

        // This could actually delay() but it's not much more difficult
        // to do it async and clean.
        // this->t_lock_output_until = (millis() + min_duration);

        delay(min_duration);

        // reset everything possible
        FastLED.setBrightness(prev_brightness);
        fill_solid(this->leds, N_LEDS, CRGB::Black);    // restore previous?
        FastLED.show();

        // make sure that OFF is visible in every situation
        delay(min_duration/4);

        // full reset is happening on next reading(display_level)
    }

    /**
     * cycle modes
     * 
     * TODO: not DRY
     * */
    void next_mode()
    {
        if (VERBOSE)
        {
            flash(CRGB::Violet);
        }

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

        // test if mode is actually missed, better usability
        if (DUALMODE)
        {
            // can't increase brightness, switch to next mode
            new_settings.brightness = USER_BRIGHTNESS::LOW_BR;

            if (this->settings.mode == USER_MODE::BAR)
            {
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
    void read(uint8_t level)
    {
        // no old levels needed
        display_level(level);

        // manage timings
        try_save_checkpoint();
    }

    // TODO: tick()?
};
