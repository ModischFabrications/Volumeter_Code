#pragma once

#include <EEPROM.h>

#include "settings.h"

#define EEPROM_MODE_ADDR 10
#define EEPROM_BRIGHTNESS_ADDR 11

// -----------

/**
 * save mode to EEPROM for persistent storage
 * EEPROM has around 100k writes per cell, so use them carefully!
 * It might be a good idea to switch cells with each new revision.
 * */
void save_settings(Settings settings)
{
  EEPROM.update(EEPROM_MODE_ADDR, (uint8_t)settings.mode);
  EEPROM.update(EEPROM_BRIGHTNESS_ADDR, (uint8_t)settings.brightness);
}

Settings load_settings()
{
  Settings settings;
  settings.mode = (Settings::MODE)EEPROM.read(EEPROM_MODE_ADDR);
  settings.brightness = (Settings::BRIGHTNESS)EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
  return settings;
}
