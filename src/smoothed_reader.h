#pragma once
/*
sampling rate varies between projects, skills and applications
that's why this class takes a sample *size* instead of time.
Keep in mind that this can really make your DATA usage explode, 
switch to a smaller data type or sample slower to reduce it.
*/

template <class INPUT_DATA_TYPE, uint16_t N_SAMPLES>
class Smoothed_Reader
{
private:
    INPUT_DATA_TYPE last_values[N_SAMPLES];

    // previous 255 samples (uint8_t) max is way too low with a target of 1s max.
    // A good ADC can reach over 100k Samples per second, most around 10k, a bad one 500.
    uint16_t next_value_addr = 0;

    // these might be implemented in some library 
    static INPUT_DATA_TYPE array_avg(INPUT_DATA_TYPE array[], uint16_t size)
    {
        INPUT_DATA_TYPE avg = 0;

        for (uint16_t i = 0; i < size; i++)
        {
            // TODO: this might round down a lot but a whole sum is not 
            // guaranteed to fit into INPUT_DATA_TYPE
            avg += (array[i] / size);
        }

        return avg;
    }

    static INPUT_DATA_TYPE array_max(INPUT_DATA_TYPE array[], uint16_t size)
    {
        INPUT_DATA_TYPE max = 0;

        for (uint16_t i = 0; i < size; i++)
        {
            INPUT_DATA_TYPE sample = array[i];
            if (sample > max) max = sample;
        }

        return max;
    }

public:
    Smoothed_Reader()
    {
        // init with zero
        for (uint16_t i = 0; i < N_SAMPLES; i++)
        {
            last_values[i] = 0;
        }
    }

    /**
     * Pass input to be smoothed out into reader
     * */
    void read(INPUT_DATA_TYPE value)
    {
        last_values[next_value_addr] = value;

        // wrapping
        this->next_value_addr++;
        if (this->next_value_addr >= N_SAMPLES)
        {
            this->next_value_addr -= N_SAMPLES;
        }
    }

    /**
     * Get the average of all previous samples
     * */
    INPUT_DATA_TYPE get_rolling_avg()
    {
        return array_avg(this->last_values, N_SAMPLES);
    }

    /**
     * Get the biggest value of the current sampling window
     * */
    INPUT_DATA_TYPE get_rolling_max()
    {
        // this could also find the last entry in max_history to save performance
        return array_max(this->last_values, N_SAMPLES);
    }
};
