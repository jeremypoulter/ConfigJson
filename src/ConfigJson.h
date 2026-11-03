#ifndef _ConfigJson_h
#define _ConfigJson_h

#include "ConfigOpt.h"
#include "ConfigOptDefinition.h"
#include "ConfigOptSecret.h"
#include "ConfigOptVirtualBool.h"
#include "ConfigOptVirtualMaskedBool.h"

// A plain function pointer rather than std::function, because the AVR core has
// no <functional>. A handler can't capture, so it keeps any state in globals.
typedef void (*ConfigJsonChangeHandler)(String name);

class ConfigJson
{
private:
  bool _modified;

  ConfigOpt **_opts;
  size_t _len;
  size_t _storage_size;
  size_t _storage_offset;
  size_t _record_size;

  ConfigJsonChangeHandler _change;
public:
  // recordSize is the most commit() may write, header included. It defaults to
  // the rest of the storage after storageOffset.
  ConfigJson(ConfigOpt **opts, size_t len, size_t storageSize, size_t storageOffset = 0, size_t recordSize = 0);

  void reset();
  bool commit();
  bool load(bool merge = false);
  // True when the stored record can't be parsed, or EEPROM can't be read at all.
  bool storedUnreadable();

  bool serialize(String& json, bool longNames = true, bool compactOutput = false, bool hideSecrets = false);
  bool serialize(Print& json, bool longNames = true, bool compactOutput = false, bool hideSecrets = false);
  bool serialize(CONFIG_JSON_DOC &doc, bool longNames = true, bool compactOutput = false, bool hideSecrets = false);

  bool deserialize(const String& json) {
    return deserialize(json.c_str());
  }
  bool deserialize(const char *json);
  bool deserialize(CONFIG_JSON_DOC &doc);

  template <typename T> bool set(const char *name, T val)
  {
#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    // Sized for a value as large as the storage, so a long string isn't cut short.
    const size_t capacity = JSON_OBJECT_SIZE(1) + _storage_size;
    DynamicJsonDocument doc(capacity);
#endif
    doc[name] = val;
    // A value that didn't fit is refused, rather than stored cut short.
    if(doc.overflowed()) {
      return false;
    }
    return ConfigJson::deserialize(doc);
  }

  bool set(const char *name, uint32_t val) {
    return set<uint32_t>(name, val);
  } 
  bool set(const char *name, int val) {
    return set<int>(name, val);
  } 
  bool set(const char *name, const String &val) {
    return set<String>(name, val);
  } 
  bool set(const char *name, bool val) {
    return set<bool>(name, val);
  } 
  bool set(const char *name, double val) {
    return set<double>(name, val);
  } 
  bool set(const String &name, uint32_t val) {
    return set<uint32_t>(name.c_str(), val);
  } 
  bool set(const String &name, int val) {
    return set<int>(name.c_str(), val);
  } 
  bool set(const String &name, const String &val) {
    return set<String>(name.c_str(), val);
  } 
  bool set(const String &name, bool val) {
    return set<bool>(name.c_str(), val);
  } 
  bool set(const String &name, double val) {
    return set<double>(name.c_str(), val);
  } 

  void onChanged(ConfigJsonChangeHandler handler) {
    _change = handler;
  }
};

#endif // _ConfigJson_h
