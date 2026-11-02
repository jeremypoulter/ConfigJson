#ifndef CONFIGJSON_TEST_SUPPORT_H
#define CONFIGJSON_TEST_SUPPORT_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ConfigOpt.h>

// Every test works in this many bytes of EEPROM, which fits all three cores.
#define TEST_STORAGE_SIZE 512

// The tests use the same EEPROM calls as ConfigJson. The AVR core has no
// begin(size) or commit(), and its writes go straight to the EEPROM.
static inline void testEepromOpen() {
#if !defined(ARDUINO_ARCH_AVR)
  EEPROM.begin(TEST_STORAGE_SIZE);
#endif
}

static inline void testEepromClose() {
#if !defined(ARDUINO_ARCH_AVR)
  EEPROM.commit();
  EEPROM.end();
#endif
}

// Blanks the test storage, so each test starts from erased EEPROM.
static inline void wipeEeprom() {
  testEepromOpen();
  for (int i = 0; i < TEST_STORAGE_SIZE; i++) {
    EEPROM.write(i, 0xff);
  }
  testEepromClose();
}

// Writes raw bytes at `address`, for records that ConfigJson wouldn't write.
static inline void pokeEeprom(int address, const uint8_t *bytes, size_t length) {
  testEepromOpen();
  for (size_t i = 0; i < length; i++) {
    EEPROM.write(address + i, bytes[i]);
  }
  testEepromClose();
}

// ConfigJson itself picks its JSON document type via CONFIG_JSON_DOC
// (JsonDocument on ArduinoJson v7, DynamicJsonDocument on v6), so the tests
// build against both majors too. Only the constructor differs -- v6's
// DynamicJsonDocument needs an explicit capacity -- so wrap it in a macro
// rather than hardcoding the v7-only default-constructible JsonDocument.
#if ARDUINOJSON_VERSION_MAJOR >= 7
#define TEST_JSON_DOC(name) CONFIG_JSON_DOC name
#else
#define TEST_JSON_DOC(name) CONFIG_JSON_DOC name(1024)
#endif

#endif
