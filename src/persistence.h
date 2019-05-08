#pragma once

#include <EEPROM.h>

#include "settings.h"

/**
 * EEPROM has around 100k writes per cell, so use them carefully!
 * It might be a good idea to switch to new cells with each revision.
 * ATtiny has 512 bytes EEPROM.
 * 
 * TODO: Preserve EEPROM while programming -> EESAVE fuse
 * */
#define EEPROM_VERSION_ADDR 12
#define EEPROM_SETTINGS_ADDR 20

// change with each design iteration to prevent EEPROM corruption
// the chance that a random combination is a match is very low
const uint8_t version = 2;

// -----------

/**
 * save mode to EEPROM for persistent storage
 * 
 * */
void save_settings(Settings settings)
{
  EEPROM.put(EEPROM_VERSION_ADDR, version);
  EEPROM.put(EEPROM_SETTINGS_ADDR, settings);
}

Settings load_settings()
{
  uint8_t saved_version;
  EEPROM.get(EEPROM_VERSION_ADDR, saved_version);

  if (saved_version != version)
  {
    // missing, corrupted or outdated, needs to be reset

    // save new settings, updates version
    Settings defaults = {Settings::MODE::BAR, Settings::BRIGHTNESS::MIDDLE_BR};
    save_settings(defaults);

    return defaults;
  }

  // content should be correct, return it
  Settings settings;
  EEPROM.get(EEPROM_SETTINGS_ADDR, settings);

  return settings;
}
