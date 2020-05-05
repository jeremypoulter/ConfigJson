#ifndef _ConfigOptDefenition_h
#define _ConfigOptDefenition_h

#include <ConfigOpt.h>

template<class T>
class ConfigOptDefenition : public ConfigOpt
{
protected:
  T &_val;
  T _default;

public:
  ConfigOptDefenition(T &v, T d, const char *l, const char *s) :
    ConfigOpt(l, s),
    _val(v),
    _default(d)
  {
  }

  T get() {
    return _val;
  }

  virtual bool set(T value) {    
    if(_val != value) {
      DBUG(_long);
      DBUG(" set to ");
      DBUGLN(value);
      _val = value;
      return true;
    }

    return false;
  }

  virtual bool serialize(DynamicJsonDocument &doc, bool longNames, bool compactOutput, bool hideSecrets) {
    if(!compactOutput || _val != _default) {
      doc[name(longNames)] = _val;
      return true;
    }

    return false;
  }

  virtual bool deserialize(DynamicJsonDocument &doc) {
    if(doc.containsKey(_long)) {
      T val = doc[_long].as<T>();
      return set(val);
    }

    if(doc.containsKey(_short)) {
      T val = doc[_short].as<T>();
      return set(val);
    }

    return true;
  }

  virtual void setDefault() {
    _val = _default;
  }
};

#endif // _ConfigOptDefenition_h
