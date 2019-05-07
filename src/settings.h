#pragma once

enum USER_MODE : uint8_t
{
  OFF = 0,
  BAR,
  DOT
};

// directly used as brightness
enum USER_BRIGHTNESS : uint8_t
{
  LOW_BR = 15,
  MIDDLE_BR = 63,
  HIGH_BR = 255
};

struct User_Settings
{
  USER_MODE mode;
  USER_BRIGHTNESS brightness;

  bool operator== (const User_Settings& s2){
    return (this->mode == s2.mode && this->brightness == s2.brightness);
  }
};