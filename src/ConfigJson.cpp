#include <Arduino.h>
#include <EEPROM.h>
#include <MicroDebug.h>

#include "ConfigJson.h"

// Stored record: a little-endian 16-bit length, then that many bytes of JSON.
static const size_t RECORD_HEADER_SIZE = 2;

// The EEPROM API differs between the cores, so the storage is only used through
// these three functions, which give the same results on all of them.
//   ESP32:   begin() returns bool, commit() returns bool, end() returns void.
//   ESP8266: begin() returns void, commit() returns bool, end() returns bool.
//   AVR:     no begin(size), commit() or end(). The EEPROM has a fixed size and
//            each write goes straight to it.

// False when the storage can't hold `size` bytes.
static bool storageBegin(size_t size)
{
#if defined(ARDUINO_ARCH_ESP32)
  if(!EEPROM.begin(size)) {
    return false;
  }
#elif defined(ARDUINO_ARCH_ESP8266)
  // begin() is void here and clamps a size it can't hold, so the length check
  // below is what tells the two apart.
  EEPROM.begin(size);
#endif
  return size <= EEPROM.length();
}

static bool storageCommit()
{
#if defined(ARDUINO_ARCH_AVR)
  return true;
#else
  return EEPROM.commit();
#endif
}

static void storageEnd()
{
#if !defined(ARDUINO_ARCH_AVR)
  EEPROM.end();
#endif
}

// True when json is a complete rendering of doc that fits the record. Out of
// memory, ArduinoJson can leave a setting null or drop it and still produce valid
// JSON, and a String that can't grow stops short. Saving either would replace a
// good stored record with a damaged one, and the loss only shows at the next boot.
static bool recordComplete(const CONFIG_JSON_DOC &doc, const String &json, size_t recordSize)
{
  size_t expected = measureJson(doc);
  return !doc.overflowed() && expected >= 2 && json.length() == expected &&
    recordSize >= RECORD_HEADER_SIZE && expected <= recordSize - RECORD_HEADER_SIZE;
}

ConfigJson::ConfigJson(ConfigOpt **opts, size_t len, size_t storageSize, size_t storageOffset, size_t recordSize) :
  _modified(false),
  _opts(opts),
  _len(len),
  _storage_size(storageSize),
  _storage_offset(storageOffset),
  _record_size(0),
  _change(nullptr)
{
  // A record must fit in the storage after the offset, and its 16-bit length
  // header can't describe more than 65535 bytes of JSON. Anything larger would
  // be written past the storage that was opened, so it is clamped here.
  size_t remaining = storageOffset < storageSize ? storageSize - storageOffset : 0;
  size_t size = recordSize ? recordSize : remaining;
  if(size > remaining) {
    size = remaining;
  }
  // Compared as unsigned long: on AVR, size_t and unsigned int are 16 bits, so
  // 0xFFFF + RECORD_HEADER_SIZE would wrap to 1 there.
  const unsigned long maxRecord = 0xFFFFUL + RECORD_HEADER_SIZE;
  if((unsigned long)size > maxRecord) {
    size = (size_t)maxRecord;
  }
  _record_size = size;
}

bool ConfigJson::load(bool merge) 
{
  bool loaded = false;

  if(false == merge) {
    reset();
  }

  if(!storageBegin(_storage_size)) {
    DBUGF("No EEPROM for config");
    return loaded;
  }

  char start = 0;
  uint8_t a = 0, b = 0;
  EEPROM.get(_storage_offset + 0, a);
  EEPROM.get(_storage_offset + 1, b);
  size_t length = a | (b << 8);

  EEPROM.get(_storage_offset + 2, start);

  DBUGF("Got %d %c from EEPROM @ 0x%04x", length, start, _storage_offset);

  if(2 <= length && length < _storage_size &&
    '{' == start)
  {
    char json[length + 1];
    for(size_t i = 0; i < length; i++) {
      json[i] = EEPROM.read(_storage_offset + 2 + i);
    }
    json[length] = '\0';
    DBUGF("Found stored JSON %s", json);
    deserialize(json);
    _modified = false;
    loaded = true;
  } else {
    DBUGF("No JSON config found");
  }

  storageEnd();

  return loaded;
}


bool ConfigJson::commit()
{
  if(false == _modified) {
    return true;
  }

  DBUGF("Saving config");

  // Rendered before EEPROM.begin(), so a low heap fails here and the stored
  // record is left untouched.
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  const size_t capacity = JSON_OBJECT_SIZE(_len) + _storage_size;
  DynamicJsonDocument doc(capacity);
#endif
  serialize(doc, false, true, false);
  String json;
  serializeJson(doc, json);

  if(!recordComplete(doc, json, _record_size)) {
    DBUGF("Config not saved: %u bytes rendered, limit %u", (unsigned)json.length(), (unsigned)_record_size);
    return false;
  }

  if(!storageBegin(_storage_size)) {
    DBUGF("Config not saved: EEPROM unavailable");
    return false;
  }

  size_t length = json.length();
  EEPROM.write(_storage_offset + 0, length & 0xff);
  EEPROM.write(_storage_offset + 1, (length >> 8) & 0xff);
  for(size_t i = 0; i < length; i++) {
    EEPROM.write(_storage_offset + RECORD_HEADER_SIZE + i, json[i]);
  }

  DBUGF("%d bytes written to EEPROM @ 0x%04x, committing", length + RECORD_HEADER_SIZE, _storage_offset);

  bool saved = storageCommit();
  storageEnd();

  if(saved) {
    DBUGF("Done");
    _modified = false;
  } else {
    DBUGF("Writing EEPROM failed");
  }
  return saved;
}

bool ConfigJson::storedUnreadable()
{
  if(!storageBegin(_storage_size)) {
    return true;
  }

  uint8_t lengthLow = EEPROM.read(_storage_offset + 0);
  uint8_t lengthHigh = EEPROM.read(_storage_offset + 1);
  char start = (char)EEPROM.read(_storage_offset + RECORD_HEADER_SIZE);

  // Same test load() uses, so the two agree on whether there is a record
  size_t length = lengthLow | ((size_t)lengthHigh << 8);
  bool present = RECORD_HEADER_SIZE <= length && length < _storage_size && '{' == start;

  String json;
  bool blank = true;
  if(present) {
    json.reserve(length);
    for(size_t i = 0; i < length; i++) {
      json += (char)EEPROM.read(_storage_offset + RECORD_HEADER_SIZE + i);
    }
  } else {
    // Erased storage has no record to protect. Anything else that isn't a
    // recognisable header is damaged, since load() can't tell it from empty.
    for(size_t i = 0; blank && i < _record_size; i++) {
      uint8_t value = EEPROM.read(_storage_offset + i);
      blank = 0x00 == value || 0xff == value;
    }
  }
  storageEnd();

  if(!present) {
    return !blank;
  }

#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  const size_t capacity = JSON_OBJECT_SIZE(_len) + _storage_size;
  DynamicJsonDocument doc(capacity);
#endif
  return DeserializationError::Ok != deserializeJson(doc, json);
}

bool ConfigJson::deserialize(const char *json) 
{
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  const size_t capacity = JSON_OBJECT_SIZE(_len) + _storage_size;
  DynamicJsonDocument doc(capacity);
#endif
  
  DeserializationError err = deserializeJson(doc, json);
  if(DeserializationError::Code::Ok == err)
  {
    ConfigJson::deserialize(doc);

    return true;
  }

  return false;
}

bool ConfigJson::deserialize(CONFIG_JSON_DOC &doc) 
{
  bool changed = false;

  for(size_t i = 0; i < _len; i++) {
    if(_opts[i]->deserialize(doc))
    {
      _modified = true;
      changed = true;
      if(_change) {
        _change(_opts[i]->name());
      }
    }
  }

  return changed;
}

bool ConfigJson::serialize(String& json, bool longNames, bool compactOutput, bool hideSecrets)
{
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  const size_t capacity = JSON_OBJECT_SIZE(30) + _storage_size;
  DynamicJsonDocument doc(capacity);
#endif

  if(ConfigJson::serialize(doc, longNames, compactOutput, hideSecrets))
  {
    serializeJson(doc, json);
    return true;
  }

  return false;
}

bool ConfigJson::serialize(Print& json, bool longNames, bool compactOutput, bool hideSecrets)
{
#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  const size_t capacity = JSON_OBJECT_SIZE(30) + _storage_size;
  DynamicJsonDocument doc(capacity);
#endif

  if(ConfigJson::serialize(doc, longNames, compactOutput, hideSecrets))
  {
    serializeJson(doc, json);
    return true;
  }

  return false;
}

bool ConfigJson::serialize(CONFIG_JSON_DOC &doc, bool longNames, bool compactOutput, bool hideSecrets)
{
  // Compact output can skip every option, which leaves the document null. That
  // serialises as "null", which load() doesn't read back as a config.
  if(doc.isNull()) {
    doc.to<JsonObject>();
  }

  for(size_t i = 0; i < _len; i++) {
    _opts[i]->serialize(doc, longNames, compactOutput, hideSecrets);
  }

  return true;
}

void ConfigJson::reset()
{
  for(size_t i = 0; i < _len; i++) {
    _opts[i]->setDefault();
  }
  _modified = true;
}

/*
template <typename T> bool ConfigJson::set(const char *name, T val)
{
  DBUG("Attempt set ");
  DBUG(name);
  DBUG(" to ");
  DBUGLN(val);

#if ARDUINOJSON_VERSION_MAJOR >= 7
  JsonDocument doc;
#else
  const size_t capacity = JSON_OBJECT_SIZE(1) + 256;
  DynamicJsonDocument doc(capacity);
#endif
  doc[name] = val;
  return ConfigJson::deserialize(doc);
}
*/
