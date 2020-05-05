#ifndef _ConfigOptSecret_h
#define _ConfigOptSecret_h

#include "ConfigOptDefenition.h"


class ConfigOptSecret : public ConfigOptDefenition<String>
{
private:
  static const char _DUMMY_PASSWORD[];

public:
  static const __FlashStringHelper * DUMMY_PASSWORD;

  ConfigOptSecret(String &v, String d, const char *l, const char *s) :
    ConfigOptDefenition<String>(v, d, l, s)
  {
  }

  bool set(String value) {
    if(value.equals(DUMMY_PASSWORD)) {
      return false;
    }

    return ConfigOptDefenition<String>::set(value);
  }

  virtual bool serialize(DynamicJsonDocument &doc, bool longNames, bool compactOutput, bool hideSecrets) {
    if(!compactOutput || _val != _default) {
      if(hideSecrets) {
        doc[name(longNames)] = (_val != 0) ? String(DUMMY_PASSWORD) : "";
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
