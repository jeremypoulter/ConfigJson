#ifndef EEPROM_H
#define EEPROM_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Real ESP8266/ESP32 EEPROM.h headers pull in Arduino.h transitively, and
// ConfigJson.cpp relies on that for String/PROGMEM -- match it here so the
// native build doesn't need its own separate Arduino.h include.
#include <Arduino.h>

// Minimal in-memory stand-in for the ESP8266/ESP32 EEPROM API -- the flavour
// ConfigJson::load()/commit() are written against -- just enough to exercise
// ConfigJson's EEPROM round-trip on the "native" PlatformIO test platform.
class EEPROMClass {
  public:
    void begin(size_t size) {
      _size = size < kCapacity ? size : kCapacity;
    }

    uint8_t read(int address) const {
      return _data[address];
    }

    void write(int address, uint8_t value) {
      _data[address] = value;
    }

    bool commit() {
      return true;
    }

    void end() {
      commit();
      _size = 0;
    }

    size_t length() const {
      return _size;
    }

    template <typename T>
    T &get(int address, T &t) const {
      memcpy((uint8_t *) &t, _data + address, sizeof(T));
      return t;
    }

    template <typename T>
    const T &put(int address, const T &t) {
      memcpy(_data + address, (const uint8_t *) &t, sizeof(T));
      return t;
    }

    // Test-only helper: wipe storage between test cases so each test starts
    // from a blank EEPROM regardless of what earlier tests committed.
    void wipe() {
      memset(_data, 0, sizeof(_data));
      _size = 0;
    }

  private:
    static const size_t kCapacity = 4096;
    uint8_t _data[kCapacity] = {};
    size_t _size = 0;
};

extern EEPROMClass EEPROM;

#endif
