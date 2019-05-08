// buildin
#include <Arduino.h>

// third-party

// private, local
#include "uv_meter.h"
#include "smoothed_reader.h"
#include "debouncer.h"

/*------------Notes-----------*\
Credits: Modisch Fabrications

Testing should be possible on the digispark, as it has an ATTiny85 onboard. 
Make sure to exclude Pin 3 and 4 as these are used for USB.

Using an ATTiny45 might be possible if you shave of a few bytes somewhere, 
but why? Save your time, it's like 10ct.

\*----------------------------*/

// --- statics, constants & defines

const bool DEBUG = true;

const uint8_t PIN_LEDS = 1;
const uint8_t PIN_BTN = 2;
const uint8_t PIN_MIC = 4;

const uint8_t N_LEDS = 12;

const uint16_t DELAY_TO_SAVE_MS = (5 * 1000);

const uint8_t N_READINGS = 30;

const uint16_t T_DEBOUNCE_MS = 50;

UV_Meter<PIN_LEDS, N_LEDS> uv_meter(DELAY_TO_SAVE_MS);

Smoothed_Reader<uint8_t, N_READINGS> reader;
Debouncer debouncer;

// --- functions

bool waveform_to_intensity(uint8_t raw_waveform)
{
  // MAX4466 has VCC/2 base level and rail-to-rail output
  // UINT8_MAX (INT8_MAX) amplitudes

  /* 5V   |    -----
          |   /     \
    2.5V  | --       -
          |           \
    0V    |            ----
          ___________________
  */

  // 17 - 127 = -110 -> 110/255 = 0.43

  uint8_t normalized_input = abs((int16_t)raw_waveform - INT8_MAX);

  return normalized_input;
}

// --------------

void setup()
{
  // init hardware

  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_MIC, INPUT);

  // init subcomponents

  // all done
  uv_meter.startup();
}

void loop()
{
  bool reading = (digitalRead(PIN_BTN) == LOW); // inverted (pullup)
  if (debouncer.read(reading))
  {
    if (DEBUG)
    {
      uv_meter.flash(CRGB::Blue, 500);
    }
    uv_meter.next_mode();
  }

  // this could be scaled (and parallelised!) by AVR register magic
  // this will block until ADC was read, which could take a while
  uint16_t raw_input = analogRead(PIN_MIC); // 10 bit ADC on ATTiny85
  uint8_t scaled_input = map(raw_input, 0, 1023, 0, 255);
  uint8_t intensity = waveform_to_intensity(scaled_input);

  reader.read(intensity);
  uv_meter.read(reader.get_rolling_avg());

  if (DEBUG)
  {
    uv_meter.flash(CRGB::DarkGreen, 100);
  }
}
