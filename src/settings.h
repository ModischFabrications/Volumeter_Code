#pragma once

// this could be 
struct Settings
{
  // directly used as global brightness modifier
  enum BRIGHTNESS : uint8_t
  {
    LOW_BR = 5,
    MIDDLE_BR = 31,
    HIGH_BR = 127,
    ULTRA_BR = 255
  };

  BRIGHTNESS brightness;

  bool operator==(const Settings &s2)
  {
    return (this->brightness == s2.brightness);
  }
};