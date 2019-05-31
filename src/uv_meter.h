#pragma once

#define FASTLED_INTERNAL // disable pragma message
#include <FastLED.h>

#include "settings.h"
#include "persistence.h"

// --- constants

const bool VERBOSE = true;
const uint16_t t_default_flash_duration = (1 * 200);

const CRGB C_OK = CRGB::White;
const CRGB C_WARN = CRGB::Yellow;
const CRGB C_CRIT = CRGB::Red;

/**
 * Display the current input value on connected RGB-LEDs. 
 * 
 * delayed persistence, mode is written to storage after "delay_to_save".
 * display is driven by input-events, no internal timer usage.
 * 
 * Timing is done with each reading, as there is nothing that needs 
 * a finer resolution. This makes `tick()` obsolete.
 * 
 * This implementation will try to ignore all mistakes. It can't 
 * really throw any exceptions as these would crash this device.
 * 
 * VERBOSE:
 * - sweep on start
 * - blink green with mode change
 * - blink green twice to confirm selection
 * */

template <uint8_t PIN_LEDS, uint16_t N_LEDS>
class UV_Meter
{
private:
    Settings settings;

    CRGB leds[N_LEDS];

    // "delayed persistence"
    const uint16_t delay_to_save_ms; // uint16_t = max 65s (to prevent dataloss)
    typedef unsigned long TIMESTAMP;
    TIMESTAMP t_next_savepoint = 0;

    /**
     * scroll bar to show power is connected and all LEDs work
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
     * update brightness, update persistence timer
     * */
    void apply_settings(Settings &new_settings, bool persistent = true)
    {
        // no need to change anything
        if (this->settings == new_settings)
            return;

        if (new_settings.brightness != this->settings.brightness)
        {
            FastLED.setBrightness(new_settings.brightness);
            // immediately show new brightness
            FastLED.show();
        }

        if (persistent)
        {
            // set "moving" timer to save as soon as user is done
            this->t_next_savepoint = (millis() + delay_to_save_ms);
        }

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
        // TODO: is this safe with overflowing values (> 1 day)?
        if (this->t_next_savepoint != 0 && millis() >= this->t_next_savepoint)
        {
            save_settings(this->settings);
            this->t_next_savepoint = 0;

            if (VERBOSE)
            {
                // once: button pressed
                // twice: confirmed and saved
                flash(CRGB::DarkGreen);
                flash(CRGB::DarkGreen);
            }
        }
    }

    uint8_t decimals_to_intensity(float value, uint8_t i)
    {
        uint8_t lower_bound = floor(value);

        /*
        x = 1.75:   0 = 255, 1 = 0.75*255, 3 = 0
        x = 1   :   0 = 255, 1 = 0
        */

        // all LEDs lower than lower bound are 1, all higher than upper 0
        if (i < lower_bound)
            return 255;
        else if (i == lower_bound) // this will be skipped with real ints
        {
            uint8_t decimals = fmod(value, 1) * UINT8_MAX; // fmod is % for floats
            return decimals;
        }
        else //if (i > lower_bound)
            return 0;
    }

    /**
     * write level to LEDs, input_level is full range of uint8_t
     * 
     * fade last LED? prevent jumping (value is decimals after division)
     * 
     * three way lerping, neighbouring pixels get value from distance
     * 
     * The color is determined by the position on the stripe and is independent 
     * of the current input_level.
     * 
     * Only the amplitude is determined by the input level. 
     * Decimals are shown as partial brightness of the last LED.
     * 
     * */
    void display_level(uint8_t input_level)
    {
        float relative_level = 1.0f * input_level / UINT8_MAX;
        float n_on = (N_LEDS * relative_level);

        // three way lerp
        fill_gradient_RGB(leds, N_LEDS, C_OK, C_WARN, C_CRIT);

        // set intensity for individual leds
        for (uint8_t i = 0; i < N_LEDS; i++)
        {
            uint8_t current_intensity = decimals_to_intensity(n_on, i);
            // scale by brightness, OFF should always be black
            // simple multiplication did not work, no idea why
            leds[i].nscale8(current_intensity);
        }

        FastLED.show();
    }

public:
    /**
     * do everything thats's okay "outside of time"
     * */
    UV_Meter(const uint16_t delay_to_save_ms, uint32_t max_milliamps = 1000) : delay_to_save_ms(delay_to_save_ms)
    {
        pinMode(PIN_LEDS, OUTPUT);

        FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(this->leds, N_LEDS);
        // set powerlimit to 5v, 1000mA (Fuse size)
        FastLED.setMaxPowerInVoltsAndMilliamps(5, max_milliamps);
    }

    /**
     * Load settings and show readiness. 
     * 
     * loads settings from persistent storage
     * */
    void startup()
    {
        if (VERBOSE)
        {
            // show first to decouple from settings
            hello_power(1 * 1000);
        }

        Settings persistent_settings = load_settings();
        apply_settings(persistent_settings, false); // no need to save back
    }

    /**
     * flash shortly to signal something.
     * Designed for debug use.
     * */
    void flash(CRGB color, uint16_t duration = t_default_flash_duration)
    {
        // decrease to prevent burning out your eyes while debugging
        uint8_t prev_brightness = FastLED.getBrightness();
        FastLED.setBrightness(32);

        fill_solid(this->leds, N_LEDS, color);
        FastLED.show();

        // This could actually delay() but it's not much more difficult
        // to do it async and clean.
        // this->t_lock_output_until = (millis() + duration);

        delay(duration * 3 / 4);

        // reset everything possible
        FastLED.setBrightness(prev_brightness);
        fill_solid(this->leds, N_LEDS, CRGB::Black); // restore previous?
        FastLED.show();

        // make sure that OFF is visible in every situation
        delay(duration / 4);

        // full reset with values is happening on next reading(display_level)
    }

    /**
     * cycle brightness
     * */
    void next_mode()
    {
        if (VERBOSE)
        {
            flash(CRGB::DarkGreen);
        }

        Settings new_settings;

        // try switching brightness first
        if (this->settings.brightness == Settings::BRIGHTNESS::LOW_BR)
        {
            new_settings = {Settings::BRIGHTNESS::MIDDLE_BR};
        }
        else if (this->settings.brightness == Settings::BRIGHTNESS::MIDDLE_BR)
        {
            new_settings = {Settings::BRIGHTNESS::HIGH_BR};
        } /*
        else if (this->settings.brightness == Settings::BRIGHTNESS::HIGH_BR)
        {
            new_settings = {Settings::BRIGHTNESS::ULTRA_BR};
        }*/
        else
        {
            new_settings = {Settings::BRIGHTNESS::LOW_BR};
        }

        apply_settings(new_settings);
    }

    /**
     * call repeatedly from main loop to animate, manage background-timings
     * */
    void read(uint8_t level)
    {
        // no old levels needed

        display_level(level);

        // manage timings
        try_save_checkpoint();
    }

    // tick()? -> overkill for uncritical checkpoints
};
