// buildin
#include <Arduino.h>

// third-party
#include <FastLED.h>

// private, local
#include "uv_meter.h"

/*------------Notes-----------*\
Credits: Modisch Fabrications

TODO: 
- move into classes
- smooth input with moving average

\*----------------------------*/

// --- statics, constants & defines

#define PIN_RGB 3
#define PIN_BTN 2
#define PIN_MIC 1

#define N_LEDS 14
CRGB leds[N_LEDS];

#define DELAY_TO_SAVE 10

// --- functions

// --------------

UV_Meter uv_meter(leds, N_LEDS, DELAY_TO_SAVE);

void setup()
{
  // init hardware
  pinMode(PIN_RGB, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, PIN_RGB, RGB>(leds, N_LEDS);

  // init subcomponents

  // all done
  uv_meter.startup();
}

void loop()
{
  // TODO check button
  //uv_meter.next_mode();

  // TODO move to input smoothing
  uv_meter.set_input_level(analogRead(PIN_MIC));
}
