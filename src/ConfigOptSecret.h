#ifndef _ConfigOptSecret_h
#define _ConfigOptSecret_h

#include "ConfigOptDefinition.h"


class ConfigOptSecret : public ConfigOptDefinition<String>
{
public:
  // Stands for a password that is set but not shown. It is a function because
  // F() isn't allowed at namespace scope on AVR.
  static const __FlashStringHelper * dummyPassword() {
    return F("_DUMMY_PASSWORD");
  }

  ConfigOptSecret(String &v, const String &d, const char *l, const char *s) :
    ConfigOptDefinition<String>(v, d, l, s)
  {
  }

  bool set(const String &value) {
    if(value.equals(dummyPassword())) {
      return false;
    }

    return ConfigOptDefinition<String>::set(value);
  }

  virtual bool serialize(CONFIG_JSON_DOC &doc, bool longNames, bool compactOutput, bool hideSecrets) {
    if(!compactOutput || _val != _default) {
      if(hideSecrets) {
        doc[name(longNames)] = (_val != 0) ? String(dummyPassword()) : "";
        return true;
      } else {
        doc[name(longNames)] = _val;
        return true;
      }
    }
    return false;
  }
};

#endif // _ConfigOptSecret_h
