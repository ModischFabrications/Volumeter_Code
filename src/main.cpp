// buildin
#include <Arduino.h>

// third-party
#include <FastLED.h>


// private, local
#include "persistence.h"
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

#define N_LEDS 14

CRGB leds[N_LEDS];

// --- functions



// --------------

UV_Meter uv_meter;

void setup() {
  // init hardware
  pinMode(PIN_RGB, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, PIN_RGB, RGB>(leds, N_LEDS);

  // init subcomponents
  uv_meter = UV_Meter(leds, N_LEDS);

  // all done
  uv_meter.init();
}

void loop() {
  // check button

}
