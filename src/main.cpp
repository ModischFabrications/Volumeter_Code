// buildin
#include <Arduino.h>

// third-party

// private, local
#include "volumeter.h"
#include "debouncer.h"

/*------------Notes-----------*\
Credits: Modisch Fabrications


\*----------------------------*/

// --- statics, constants & defines

#ifdef ARDUINO_AVR_NANO
#pragma message "Enabling Serial for Debug Prints"
const bool USE_SERIAL = true;

const uint8_t N_LEDS = 6;
const uint8_t PIN_LEDS = 4;

#else
const bool USE_SERIAL = false;

const uint8_t N_LEDS = 12;
const uint8_t PIN_LEDS = 1;
#endif

const uint8_t PIN_BTN = 2;
const uint8_t PIN_MIC = A2;

const uint16_t DELAY_TO_SAVE_MS = (5 * 1000);
const uint32_t MAX_MILLIAMPS = 500;

const float ADJUSTMENT_FACTOR = 1.8f;

Volumeter<PIN_LEDS, N_LEDS> volumeter(DELAY_TO_SAVE_MS, MAX_MILLIAMPS);

Debouncer debouncer;

// --- functions

uint16_t waveform_to_amplitude(uint16_t raw_waveform)
{
  /*
    MAX4466 has VCC/2 base level and rail-to-rail output
    Input: 512 +/- 512
    Output: 0..512

     5V   |    -----
          |   /     \
    2.5V  | --       -
          |           \
    0V    |            ----
          ___________________
  */

  // 17 - 127 = -110 -> 110/255 = 0.43

  return abs((int16_t)raw_waveform - 512);
}

/**
 * This is a simple and efficient trick to keep a rolling average.
 * Keep in mind that this is coupled to the execution time, make sure 
 * to keep it predictable.
 * 
 * */
uint8_t simple_avg(uint8_t new_value, float factor = 0.95f)
{
  static uint16_t last_value = 0;

  // rescale input to higher precision
  uint16_t scaled_new = (new_value << 8);

  // use uint16_t internally to keep high precision for small deltas
  uint16_t current_value = (last_value * factor) + ((1 - factor) * scaled_new);
  last_value = current_value;

  // scale back to lower precision
  return (current_value >> 8);
}

// --------------

void setup()
{
  // init hardware

  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_MIC, INPUT_PULLUP);

  // init subcomponents

  if (USE_SERIAL)
  {
    Serial.begin(115200);
  }

  // all done
  volumeter.startup();
}

void loop()
{
  bool reading = (digitalRead(PIN_BTN) == LOW); // inverted (pullup)
  if (debouncer.read(reading) == Debouncer::STATE::ST_RISING)
  {
    volumeter.next_mode();
  }

  // you could increase speed *a lot* by triggering ADC-read via a HW-timer
  // or manually at the start of each loop after reading it's buffer
  uint16_t mic_reading = analogRead(PIN_MIC); // 512 +/- 512
  // rescale to stay inside defined boundaries
  uint8_t amplitude = waveform_to_amplitude(mic_reading) / 2; // 0..255
  uint8_t average_amplitude = simple_avg(amplitude);

  uint8_t scaled_average_amplitude = (ADJUSTMENT_FACTOR * average_amplitude);
  // detect (and fix) overflow
  if (scaled_average_amplitude < average_amplitude)
    scaled_average_amplitude = UINT8_MAX;

  if (USE_SERIAL)
  {
    Serial.print(amplitude);
    Serial.print(",");
    Serial.print(average_amplitude);
    Serial.print(",");
    Serial.print(scaled_average_amplitude);
    Serial.println();
  }

  volumeter.read(scaled_average_amplitude);

  FastLED.delay(10); // keep a predictable execution time
}
