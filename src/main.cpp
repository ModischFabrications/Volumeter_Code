// buildin
#include <Arduino.h>

// third-party

// private, local
#include "uv_meter.h"
#include "smoothed_reader.h"

/*------------Notes-----------*\
Credits: Modisch Fabrications

Testing should be possible on the digispark, as it has an ATTiny85 onboard. 
Make sure to exclude Pin 3 and 4 as these are used for USB.

Using an ATTiny45 might be possible if you shave of a few bytes somewhere, 
but why? Save your time, it's like 10ct.

\*----------------------------*/

// --- statics, constants & defines

#define PIN_LEDS 1
/* 
PIN 7 is the only hardware interrupt on ATTiny85 (INT0)
but ISRs are overkill in this case and are ignored
`attachInterrupt(0, wakeUpFunction, LOW);`
*/

const uint8_t PIN_BTN = 2;
const uint8_t PIN_MIC = 4;

const uint8_t N_LEDS = 14;
const uint8_t N_THRESHOLD_YELLOW = (N_LEDS * 0.5);
const uint8_t N_THRESHOLD_RED = (N_LEDS * 0.8);

const uint16_t DELAY_TO_SAVE_MS = (5 * 1000);

const uint8_t N_READINGS = 30;

const uint16_t T_DEBOUNCE_MS = 50;

UV_Meter<PIN_LEDS, N_LEDS> uv_meter(DELAY_TO_SAVE_MS, N_THRESHOLD_YELLOW, N_THRESHOLD_RED);
 
Smoothed_Reader<uint8_t, N_READINGS> reader;

// --- functions

bool check_button_debounced(bool button_state)
{
  // this function will not trigger for the first
  // flank but for the "last", which should be stable

  static unsigned long last_debounce = 0;
  static bool last_state = true;

  unsigned long now = millis();

  if (button_state != last_state)
  {
    // recent change means it's still bouncing or somebody clicked
    last_debounce = now;
    return false;
  }

  if ((now - last_debounce) > T_DEBOUNCE_MS)
  {
    // value is old enough to be stable

    // check if a real change has occured
    if (button_state != last_state)
    {
      last_state = button_state;

      // this implementation only cares for negative edges
      if (!button_state)
      {
        return true;
      }
    }
  }
  return false;
}

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
  bool button_state = (digitalRead(PIN_BTN) == LOW); // inverted (pullup)
  if (check_button_debounced(button_state))
  {
    uv_meter.next_mode();
  }

  uint16_t raw_input = analogRead(PIN_MIC); // 10 bit ADC on ATTiny85
  uint8_t scaled_input = map(raw_input, 0, 1023, 0, 255);
  uint8_t intensity = waveform_to_intensity(scaled_input);

  reader.read(intensity);
  uv_meter.read(reader.get_rolling_avg());

  delay(1); // ADC minimum, could be even lower
}
