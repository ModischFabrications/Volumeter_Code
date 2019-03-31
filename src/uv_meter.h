#pragma once




class UV_Meter{
private:
    User_Settings settings;

    CRGB *leds;
    const uint16_t n_leds;




    /**
     * blink a bit to show power is connected
     * (current implementation scrolls bar)
     * */
    void hello_power(){
        for (uint8_t i=0; i<n_leds; i++){
            leds[i] = CRGB::White;
            FastLED.show();
            delay(1000/n_leds);  // 1 second for whole bar
            leds[i] = CRGB::Black;
        }
    }


public:

    UV_Meter(CRGB* leds, uint16_t n_leds) : leds(leds), n_leds(n_leds){
        settings = load_settings();
    }

    void init(){
        hello_power();
        // set afterwards to prevent hiding startup
        FastLED.setBrightness(settings.brightness);
    }

    /**
     * the button was pressed
     * */
    void trigger_input(){
        // TODO
    }
};
