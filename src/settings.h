#pragma once

struct Settings
{
  enum MODE : uint8_t
  {
    OFF = 0,
    BAR,
    DOT
  };

  // directly used as brightness
  enum BRIGHTNESS : uint8_t
  {
    LOW_BR = 15,
    MIDDLE_BR = 63,
    HIGH_BR = 255
  };

  MODE mode;
  BRIGHTNESS brightness;

  bool operator==(const Settings &s2)
  {
    return (this->mode == s2.mode && this->brightness == s2.brightness);
  }
};