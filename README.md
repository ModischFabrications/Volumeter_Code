# UV_Meter_Code
submodule for code of UV-Meter

PlatformIO is used to manage code, see master module for more informations.

# Design considerations

## Input

### User
Input: 1 Button, toggles modes

Modes are permutations of:
Bar/Dot | white/gradient/gradient_at_pos | low/high

gradient: [green, yellow, red]

### Microphone

The microphone transmits audio signals as an analog signal [0-VCC] with baseline VCC/2.

## Output

Strip of high density RGB-LEDs (144LED/m WS2812B).
Library used: `FastLED`

## Implementation:
Write current mode after 10s (ACK) to EEPROM
`EEPROM.update(address, val);`


