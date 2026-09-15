# ConfigJson

[![Platform IO](https://github.com/jeremypoulter/ConfigJson/actions/workflows/platformio.yml/badge.svg)](https://github.com/jeremypoulter/ConfigJson/actions/workflows/platformio.yml)
[![Unit Tests](https://github.com/jeremypoulter/ConfigJson/actions/workflows/tests.yml/badge.svg)](https://github.com/jeremypoulter/ConfigJson/actions/workflows/tests.yml)

Save application configuration to EEPROM as a minimised JSON document, for
Arduino-family boards (AVR, ESP8266, ESP32, ...).

Rather than hand-writing EEPROM offsets for every setting, you describe each
option once -- its type, a default, and a long/short name -- and `ConfigJson`
handles loading, saving, and JSON (de)serialisation for the whole set.

## Features

- Define each setting once and get EEPROM load/save, JSON serialisation and
  JSON parsing for free.
- Every option has both a long name (e.g. `"password"`) and a short name
  (e.g. `"p"`), so a UI can show friendly labels while the JSON stored in
  EEPROM stays compact.
- Compact serialisation mode omits any option still at its default value.
- Secret options (`ConfigOptSecret`, e.g. WiFi passwords) can be serialised
  with their value masked, so a config dump is safe to show in a UI or log.
- Virtual boolean options (`ConfigOptVirtualBool`) pack several flags into a
  single stored integer, one bit each, without needing their own EEPROM
  storage.
- `onChanged()` gets called with the name of each option a JSON document
  actually modified, so you can react to individual settings changing.

## Installation

ConfigJson is published to the [PlatformIO registry](https://registry.platformio.org/libraries/jeremypoulter/ConfigJson).
Add it to your project's `platformio.ini`:

```ini
lib_deps =
  ConfigJson
```

It depends on [ArduinoJson](https://arduinojson.org/) (>=6.16.1, including
v7) and [Micro Debug](https://github.com/jeremypoulter/MicroDebug), both of
which PlatformIO will pull in automatically.

## Quick start

```cpp
#include <Arduino.h>
#include <ConfigJson.h>

#define EEPROM_SIZE 512

String deviceName;
int32_t brightness;
bool enabled;

ConfigOpt *opts[] = {
  new ConfigOptDefinition<String>(deviceName, "lamp", "name", "n"),
  new ConfigOptDefinition<int32_t>(brightness, 128, "brightness", "b"),
  new ConfigOptDefinition<bool>(enabled, true, "enabled", "e"),
};

ConfigJson config(opts, sizeof(opts) / sizeof(opts[0]), EEPROM_SIZE);

void setup() {
  Serial.begin(115200);

  // Loads from EEPROM if a valid document is found there, otherwise leaves
  // every option at its default.
  config.load();

  // Change a setting and persist it.
  config.set("brightness", 200);
  config.commit();

  // Dump the current config as JSON, e.g. for a web UI.
  String json;
  config.serialize(json);
  Serial.println(json);
}

void loop() {}
```

See [`examples/load_config`](examples/load_config) for a runnable version of
this, and [`examples/virtual_bool_flags`](examples/virtual_bool_flags) for
packing several booleans into one stored value.

## Option types

Every option wraps a variable in your sketch and knows how to read/write it
from/to JSON and EEPROM.

| Class | Use for |
| --- | --- |
| `ConfigOptDefinition<T>` | A plain value: `String`, `bool`, `int32_t`, `uint32_t`, `double`, ... |
| `ConfigOptSecret` | A `String` that should be masked when serialised with `hideSecrets=true` (passwords, tokens). |
| `ConfigOptVirtualBool` | A single bit inside a `ConfigOptDefinition<uint32_t>`, exposed as its own named boolean option. |
| `ConfigOptVirtualMaskedBool` | Like `ConfigOptVirtualBool`, but also records which bits have ever been explicitly set, in a second `ConfigOptDefinition<uint32_t>`. |

```cpp
// A plain option.
new ConfigOptDefinition<int32_t>(brightness, 128, "brightness", "b");

// A secret, masked when serialize(..., hideSecrets=true) is used.
new ConfigOptSecret(wifiPassword, "", "password", "p");

// Two named flags packed into the low two bits of one uint32_t.
uint32_t flags = 0;
ConfigOptDefinition<uint32_t> flagsOpt(flags, 0, "flags", "f");
new ConfigOptVirtualBool(flagsOpt, 0x01, 0x01, "soundEnabled", "se");
new ConfigOptVirtualBool(flagsOpt, 0x02, 0x00, "ledDisabled", "ld");
```

## `ConfigJson` API

```cpp
ConfigJson(ConfigOpt **opts, size_t len, size_t storageSize, size_t storageOffset = 0);
```
Wraps an array of options. `storageSize`/`storageOffset` describe the region
of EEPROM the serialised JSON is stored in (2 length bytes followed by the
JSON text).

- `bool load(bool merge = false)` -- reads and applies the JSON stored in
  EEPROM. Unless `merge` is `true`, every option is reset to its default
  first. Returns `false` if no valid document was found (options are still
  left at their defaults/current values).
- `void commit()` -- writes the current options to EEPROM as JSON, if
  anything has changed since the last `load()`/`commit()`.
- `void reset()` -- sets every option back to its default.
- `bool serialize(String &json, bool longNames = true, bool compactOutput = false, bool hideSecrets = false)`
  and the `Print&`/`JsonDocument&` overloads -- write the current options out
  as JSON.
- `bool deserialize(const char *json)` / `bool deserialize(JsonDocument &doc)`
  -- apply a JSON document to the matching options. Returns `true` if
  anything changed.
- `bool set(const char *name, T val)` -- convenience for changing a single
  option by name (works for `String`, `bool`, `int`, `uint32_t`, `double`).
- `void onChanged(std::function<void(String name)> handler)` -- called once
  per option that a `deserialize()`/`set()` call actually changed.

## Testing

Unit tests live in [`tests/unit`](tests/unit) and run on PlatformIO's
`native` platform (no hardware required), using
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) for the Arduino
`String`/`Print`/EEPROM APIs on the host. The same suite runs against both
ArduinoJson v6 and v7 -- matching the `>=6.16.1` range ConfigJson supports --
via two PlatformIO envs:

```sh
cd tests/unit
pio test -e native      # ArduinoJson v7
pio test -e native_v6   # ArduinoJson v6
```

Every push and pull request runs both in CI (see
[`.github/workflows/tests.yml`](.github/workflows/tests.yml)), which also
reports results and line coverage (from the v7 run) on the pull request.
