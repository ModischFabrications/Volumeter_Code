// buildin
#include <Arduino.h>

// third-party


// private, local
#include "uv_meter.h"
#include "smoothed_reader.h"

/*------------Notes-----------*\
Credits: Modisch Fabrications

TODO: 
- move into classes
- smooth input with moving average

\*----------------------------*/

// --- statics, constants & defines

#define PIN_LEDS 3
#define PIN_BTN 2
#define PIN_MIC 1

#define N_LEDS 14

#define DELAY_TO_SAVE 10

#define N_READINGS 30

// --- functions



// --------------

UV_Meter <PIN_LEDS, N_LEDS> uv_meter(DELAY_TO_SAVE);
Smoothed_Reader <uint8_t, N_READINGS> reader(PIN_MIC);

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
  // TODO check button
  //uv_meter.next_mode();

  // TODO move to input smoothing
  uv_meter.set_input_level(reader.get_rolling_avg());

  delay(1); // ADC minimum
}
