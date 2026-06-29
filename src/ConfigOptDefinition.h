#ifndef _ConfigOptDefinition_h
#define _ConfigOptDefinition_h

#include <ConfigOpt.h>

#define ConfigOptDefenition ConfigOptDefinition

template<class T>
class ConfigOptDefinition : public ConfigOpt
{
protected:
  T &_val;
  T _default;

public:
  ConfigOptDefinition(T &v, const T &d, const char *l, const char *s) :
    ConfigOpt(l, s),
    _val(v),
    _default(d)
  {
  }

  T get() {
    return _val;
  }

  virtual bool set(const T &value) {    
    if(_val != value) {
      _val = value;
      return true;
    }

    return false;
  }

  virtual bool serialize(CONFIG_JSON_DOC &doc, bool longNames, bool compactOutput, bool hideSecrets) {
    if(!compactOutput || _val != _default) {
      doc[name(longNames)] = _val;
      return true;
    }

    return false;
  }

  virtual bool deserialize(CONFIG_JSON_DOC &doc) {
#if ARDUINOJSON_VERSION_MAJOR >= 7
    if(doc[_long].template is<T>()) {
#else
    if(doc.containsKey(_long)) {
#endif
      T val = doc[_long].template as<T>();
      return set(val);
    }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    if(doc[_short].template is<T>()) {
#else
    if(doc.containsKey(_short)) {
#endif
      T val = doc[_short].template as<T>();
      return set(val);
    }

    return false;
  }

  virtual void setDefault() {
    _val = _default;
  }
};

#endif // _ConfigOptDefinition_h
