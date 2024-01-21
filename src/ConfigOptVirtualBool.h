#ifndef _ConfigOptVirtualBool_h
#define _ConfigOptVirtualBool_h

#include "ConfigOpt.h"
#include "ConfigOptDefinition.h"

class ConfigOptVirtualBool : public ConfigOpt
{
protected:
  ConfigOptDefinition<uint32_t> &_base;
  uint32_t _mask;
  uint32_t _expected;

public:
  ConfigOptVirtualBool(ConfigOptDefinition<uint32_t> &b, uint32_t m, uint32_t e, const char *l, const char *s) :
    ConfigOpt(l, s),
    _base(b),
    _mask(m),
    _expected(e)
  {
  }

  bool get() {
    return _expected == (_base.get() & _mask);
  }

  virtual bool set(bool value) {
    uint32_t newVal = _base.get() & ~(_mask);
    if(value == (_mask == _expected)) {
      newVal |= _mask;
    }
    return _base.set(newVal);
  }

  virtual bool serialize(DynamicJsonDocument &doc, bool longNames, bool compactOutput, bool hideSecrets) {
    if(!compactOutput) {
      doc[name(longNames)] = get();
      return true;
    }
    return false;
  }

  virtual bool deserialize(DynamicJsonDocument &doc) {
    if(doc.containsKey(_long)) {
      return set(doc[_long].as<bool>());
    } else if(doc.containsKey(_short)) { \
      return set(doc[_short].as<bool>());
    }

    return false;
  }

  virtual void setDefault() {
  }
};

#endif // _ConfigOptVirtualBool_h
