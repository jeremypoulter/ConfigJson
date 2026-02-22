#ifndef _ConfigOpt_h
#define _ConfigOpt_h

#include <ArduinoJson.h>

#if ARDUINOJSON_VERSION_MAJOR >= 7
  #define CONFIG_JSON_DOC JsonDocument
#else
  #define CONFIG_JSON_DOC DynamicJsonDocument
#endif

class ConfigOpt;
template<class T> class ConfigOptDefinition;

class ConfigOpt
{
protected:
  const char *_long;
  const char *_short;
public:
  ConfigOpt(const char *l, const char *s) :
    _long(l),
    _short(s)
  {
  }

  const char *name(bool longName = true) {
    return longName ? _long : _short;
  }

  virtual bool serialize(CONFIG_JSON_DOC &doc, bool longNames, bool compactOutput, bool hideSecrets) = 0;
  virtual bool deserialize(CONFIG_JSON_DOC &doc) = 0;
  virtual void setDefault() = 0;

  template <typename T> bool set(T val) {
    ConfigOptDefinition<T> *opt = (ConfigOptDefinition<T> *)this;
    return opt->set(val);
  }
};

#endif // _ConfigOpt_h
