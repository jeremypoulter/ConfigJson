#ifndef _ConfigJson_h
#define _ConfigJson_h

#include "ConfigOpt.h"
#include "ConfigOptDefenition.h"
#include "ConfigOptSecret.h"
#include "ConfigOptVirtualBool.h"

class ConfigJson
{
private:
  bool _modified;

  ConfigOpt **_opts;
  size_t _len;
  size_t _storage_size;
public:
  ConfigJson(ConfigOpt **opts, size_t len, size_t storageSize);

  void reset();
  void commit();
  bool load();

  bool serialize(String& json, bool longNames = true, bool compactOutput = false, bool hideSecrets = false);
  bool serialize(Print& json, bool longNames = true, bool compactOutput = false, bool hideSecrets = false);
  bool serialize(DynamicJsonDocument &doc, bool longNames = true, bool compactOutput = false, bool hideSecrets = false);

  bool deserialize(String& json) {
    return deserialize(json.c_str());
  }
  bool deserialize(const char *json);
  bool deserialize(DynamicJsonDocument &doc);

  template <typename T> bool set(const char *name, T val)
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

  bool set(const char *name, uint32_t val) {
    return set<uint32_t>(name, val);
  } 
  bool set(const char *name, String val) {
    return set<String>(name, val);
  } 
  bool set(const char *name, bool val) {
    return set<bool>(name, val);
  } 
  bool set(const char *name, double val) {
    return set<double>(name, val);
  } 

};

#endif // _ConfigJson_h
