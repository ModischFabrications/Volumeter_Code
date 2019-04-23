#pragma once

#include <Arduino.h>


template <class INPUT_DATA_TYPE, uint8_t SAMPLE_SIZE> class Smoothed_Reader
{
private:
    const uint8_t pin_to_read;

    INPUT_DATA_TYPE last_values[SAMPLE_SIZE];          // average over last readings
    uint8_t next_writable_addr = 0;


    void read_input()
    {
        last_values[next_writable_addr] = analogRead(pin_to_read);

        // wrapping
        this->next_writable_addr++;
        if (this->next_writable_addr >= SAMPLE_SIZE) 
        {
            this->next_writable_addr -= SAMPLE_SIZE;
        }
    }

public:
    Smoothed_Reader(const uint8_t pin_to_read) : pin_to_read(pin_to_read){
        
        // init with zero
        for (uint8_t i = 0; i < SAMPLE_SIZE; i++)
        {
            last_values[i] = 0;
        }
    }

    INPUT_DATA_TYPE get_rolling_avg()
    {
        INPUT_DATA_TYPE rolling_avg = 0;

        for (uint8_t i = 0; i < SAMPLE_SIZE; i++)
        {
            rolling_avg += (last_values[i]/SAMPLE_SIZE);
        }

        return rolling_avg;
    }

    void tick()
    {
        read_input();
    }
};

