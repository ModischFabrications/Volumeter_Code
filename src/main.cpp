// buildin
#include <Arduino.h>
#include <EEPROM.h>

// third-party
#include <FastLED.h>

// --- Notes
/*---------------*\
Credits: Modisch Fabrications

TODO: move into classes

\*---------------*/

// --- statics, constants & defines

#define PIN_RGB 3
#define PIN_BTN 2

#define N_LEDS 14

CRGB leds[N_LEDS];

#define EEPROM_MODE_ADDR 10
#define EEPROM_BRIGHTNESS_ADDR 11

enum USER_MODE{
  OFF,
  BAR,
  DOT
};

// directly used as brightness
enum USER_BRIGHTNESS{
  LOW_BR = 15,
  NORMAL_BR = 63,
  HIGH_BR = 255
};

struct User_Settings{
  USER_MODE mode;
  USER_BRIGHTNESS brightness;
};

// --- functions


/**
 * save mode to EEPROM for persistent storage
 * EEPROM has around 100k writes per cell, so use them carefully!
 * It might be a good idea to switch cells with each new revision.
 * */
void save_settings(User_Settings settings){
  EEPROM.update(EEPROM_MODE_ADDR, (uint8_t)settings.mode);
  EEPROM.update(EEPROM_BRIGHTNESS_ADDR, (uint8_t)settings.brightness);
}

User_Settings load_settings(){
  User_Settings settings;
  settings.mode =(USER_MODE) EEPROM.read(EEPROM_MODE_ADDR);
  settings.brightness =(USER_BRIGHTNESS) EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
  return settings;
}

/**
 * blink a bit to show power is connected
 * (current implementation scrolls bar)
 * */
void hello_power(){
  for (uint8_t i=0; i<N_LEDS; i++){
    leds[i] = CRGB::White;
    FastLED.show();
    delay(1000/N_LEDS);  // 1 second for whole bar
    leds[i] = CRGB::Black;
  }
}

// --------------

void setup() {
  // init hardware
  pinMode(PIN_RGB, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, PIN_RGB, RGB>(leds, N_LEDS);

  // init subcomponents
  User_Settings curr_settings = load_settings();
  FastLED.setBrightness(curr_settings.brightness);

  // all done
  hello_power();
}

void loop() {
  // put your main code here, to run repeatedly:
}
