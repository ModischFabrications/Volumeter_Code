#pragma once

#include <Arduino.h>

const uint16_t DEF_DEBOUNCE_MS = 50;

class Debouncer
{
private:
    const uint16_t debounce_ms;

    uint32_t last_debounce = 0;
    bool last_state = false;
    bool current_state = false;

public:
    enum STATE
    {
        ST_LOW = -2,
        ST_RISING = -1,
        UNSTABLE = 0,
        ST_HIGH = 2,
        ST_FALLING = 1
    };

    Debouncer(uint16_t debounce_ms = DEF_DEBOUNCE_MS)
        : debounce_ms(debounce_ms)
    {
    }

    STATE read(bool reading)
    {
        uint32_t now = millis(); // unsigned long

        if (reading != last_state)
        {
            // recent change means it's still bouncing or somebody clicked
            // -> this reading can't be trusted
            last_debounce = now;
            //return false;
        }

        if ((now - last_debounce) > debounce_ms)
        {
            // value is old enough to be stable

            if (reading != current_state)
            {
                // this was a flank, act only once
                current_state = reading;

                if (current_state)
                {
                    return STATE::ST_RISING;
                }
                else
                {
                    return STATE::ST_FALLING;
                }
            }
            else
            {
                if (current_state)
                {
                    return STATE::ST_HIGH;
                }
                else
                {
                    return STATE::ST_LOW;
                }
            }
        }

        last_state = reading;

        return STATE::UNSTABLE;
    }
};
