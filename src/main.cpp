// buildin
#include <Arduino.h>

// third-party


// private, local
#include "uv_meter.h"
#include "smoothed_reader.h"

/*------------Notes-----------*\
Credits: Modisch Fabrications

TODO: 

\*----------------------------*/

// --- statics, constants & defines

#define PIN_LEDS PB1
// PIN 7 is the only hardware interrupt on ATTiny85 (INT0)
// attachInterrupt(0,wakeUpFunction, LOW);
// (ISR are overkill in this case and are ignored)
#define PIN_BTN PB2
#define PIN_MIC PB4

#define N_LEDS 14

#define DELAY_TO_SAVE 10

#define N_READINGS 30

#define T_DEBOUNCE_MS 50

UV_Meter <PIN_LEDS, N_LEDS> uv_meter(DELAY_TO_SAVE);
Smoothed_Reader <uint8_t, N_READINGS> reader(PIN_MIC);

// --- functions


void check_button_debounced(){
  static unsigned long last_debounce = 0;
  static bool last_state = HIGH;

  unsigned long now = millis();

  bool button_state = digitalRead(PIN_BTN);

  // recent change means it's still bouncing or somebody clicked
  if (button_state != last_state){
    last_debounce = now;
  }

  if ( (now - last_debounce) > T_DEBOUNCE_MS) {
    // value is old enough to be stable
 
    // check if a real change has occured
    if (button_state != last_state){
      last_state = button_state;

      // this implementation only cares for positive edges
      if (button_state == LOW){
        uv_meter.next_mode();
      }
    }   
  }
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
  // TODO: this is ugly and could be better
  // calls uv_meter.next_mode internally
  check_button_debounced();

  uv_meter.set_input_level(reader.get_rolling_avg());

  delay(1); // ADC minimum
}
