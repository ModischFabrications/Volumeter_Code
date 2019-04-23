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

#define PIN_BTN 2
#define PIN_MIC 4

#define N_LEDS 14

#define DELAY_TO_SAVE 10

#define N_READINGS 30

#define T_DEBOUNCE_MS 50

UV_Meter<PIN_LEDS, N_LEDS> uv_meter(DELAY_TO_SAVE);
Smoothed_Reader<uint8_t, N_READINGS> reader(PIN_MIC);

// --- functions

bool check_button_debounced()
{
  // this function will not trigger for the first
  // flank but for the "last", which should be stable

  static unsigned long last_debounce = 0;
  static bool last_state = HIGH;

  unsigned long now = millis();

  bool button_state = digitalRead(PIN_BTN);

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

      // this implementation only cares for positive edges
      if (button_state == LOW)
      {
        return true;
      }
    }
  }
  return false;
}

// --------------

void setup()
{
  // init hardware

  pinMode(PIN_BTN, INPUT_PULLUP);

  // init subcomponents

  // all done
  uv_meter.startup();
}

void loop()
{
  if (check_button_debounced())
  {
    uv_meter.next_mode();
  }

  uv_meter.set_input_level(reader.get_rolling_avg());

  delay(1); // ADC minimum
}
