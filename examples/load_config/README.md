# load_config

A minimal sketch showing the core `ConfigJson` workflow: define a `String`,
an `int32_t`, a `bool` and a masked secret, load whatever is stored in
EEPROM (or fall back to defaults), then print the config back out in a few
different forms -- long names, short names, compact (defaults omitted), and
with the secret hidden.

Build for a Huzzah (ESP8266) board:

```sh
pio run -e huzzah
```
