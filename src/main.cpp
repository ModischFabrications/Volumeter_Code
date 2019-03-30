// buildin
#include <Arduino.h>
#include <EEPROM.h>

// third-party
#include <FastLED.h>


// --- statics, constants & defines

#define PIN_RGB 3
#define PIN_BTN 2

#define N_LEDS 14

CRGB led_strip[N_LEDS];

#define EEPROM_MODE_ADDR 10

enum USER_MODE{
  OFF,
  BAR,
  DOT
};

// --- functions

/**
 * blink a bit to show power is connected
 * 
 * */
void hello(){
  
}

void set_mode(USER_MODE mode){

}

/**
 * save mode to EEPROM for persistent storage
 * EEPROM has around 100k writes per cell, so use them carefully!
 * */
void save_mode(USER_MODE mode){
  EEPROM.update(EEPROM_MODE_ADDR, (uint8_t)mode);
}

USER_MODE load_mode(){
  return (USER_MODE) EEPROM.read(EEPROM_MODE_ADDR);
}

// --------------

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_RGB, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, PIN_RGB, RGB>(led_strip, N_LEDS);
  FastLED.show();
}

void loop() {
  // put your main code here, to run repeatedly:
}