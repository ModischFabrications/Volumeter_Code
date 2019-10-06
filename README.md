# Volumeter_Code
submodule for code of Volumeter

PlatformIO is used to manage code, see master module for more informations.

## Design considerations

### Input

#### User
Input: 1 Button, step through brightness with wraparound

#### Microphone

The microphone transmits audio signals as an analog signal [0-VCC] with baseline VCC/2.

### Output

Strip of high density RGB-LEDs (144LED/m WS2812B).
Library used: `FastLED`

gradient: green->yellow->red

### Implementation:
Write current mode after 10s (ACK) to EEPROM, read on startup

## Uploading
This code assumes an Arduino Nano as debugger or an Attiny85 for the final version.
Serial communication and ports are adjusted automatically, just hit "upload".
