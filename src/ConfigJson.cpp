#include <EEPROM.h>
#include <MicroDebug.h>

#include "ConfigJson.h"

const char ConfigOptSecret::_DUMMY_PASSWORD[] PROGMEM = "_DUMMY_PASSWORD";
const __FlashStringHelper * ConfigOptSecret::DUMMY_PASSWORD = FPSTR(_DUMMY_PASSWORD);

ConfigJson::ConfigJson(ConfigOpt **opts, size_t len, size_t storageSize) :
  _opts(opts),
  _len(len),
  _storage_size(storageSize)
{
}

bool ConfigJson::load() 
{
  bool loaded = true;

  reset();

  EEPROM.begin(_storage_size);

  char start = 0;
  uint8_t a = 0, b = 0;
  EEPROM.get(0, a);
  EEPROM.get(1, b);
  int length = a | (b << 8);

  EEPROM.get(2, start);

  DBUGF("Got %d %c from EEPROM", length, start);

  if(2 <= length && length < EEPROM.length() &&
    '{' == start)
  {
    char json[length + 1];
    for(int i = 0; i < length; i++) {
      json[i] = EEPROM.read(2+i);
    }
    json[length] = '\0';
    DBUGF("Found stored JSON %s", json);
    deserialize(json);
    _modified = false;
    loaded = true;
  } else {
    DBUGF("No JSON config found");
  }

  EEPROM.end();

  return loaded;
}


void ConfigJson::commit()
{
  if(false == _modified) {
    return;
  }

  DBUGF("Saving config");
  
  EEPROM.begin(_storage_size);

  String jsonStr;
  ConfigJson::serialize(jsonStr, false, true, false);
  const char *json = jsonStr.c_str();
  DBUGF("Writing %s to EEPROM", json);
  int length = jsonStr.length();
  EEPROM.put(0, length & 0xff);
  EEPROM.put(1, (length >> 8) & 0xff);
  for(int i = 0; i < length; i++) {
    EEPROM.write(2+i, json[i]);
  }

  DBUGF("%d bytes written to EEPROM, committing", length + 2);

  if(EEPROM.commit())
  {
    DBUGF("Done");
    _modified = false;
  } else {
    DBUGF("Writting EEPROM failed");
  }
}

bool ConfigJson::deserialize(const char *json) 
{
  const size_t capacity = JSON_OBJECT_SIZE(_len) + EEPROM.length();
  DynamicJsonDocument doc(capacity);
  
  DeserializationError err = deserializeJson(doc, json);
  if(DeserializationError::Code::Ok == err)
  {
    ConfigJson::deserialize(doc);

    return true;
  }

  return false;
}

bool ConfigJson::deserialize(DynamicJsonDocument &doc) 
{
  bool changed = false;

  for(size_t i = 0; i < _len; i++) {
    if(_opts[i]->deserialize(doc))
    {
      _modified = true;
      changed = true;
    }
  }

  return changed;
}

bool ConfigJson::serialize(String& json, bool longNames, bool compactOutput, bool hideSecrets)
{
  const size_t capacity = JSON_OBJECT_SIZE(30) + EEPROM.length();
  DynamicJsonDocument doc(capacity);

  if(ConfigJson::serialize(doc, longNames, compactOutput, hideSecrets))
  {
    serializeJson(doc, json);
    return true;
  }

  return false;
}

bool ConfigJson::serialize(DynamicJsonDocument &doc, bool longNames, bool compactOutput, bool hideSecrets)
{
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

  const size_t capacity = JSON_OBJECT_SIZE(1) + 256;
  DynamicJsonDocument doc(capacity);
  doc[name] = val;
  return ConfigJson::deserialize(doc);
}
*/
