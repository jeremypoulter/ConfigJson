# virtual_bool_flags

Shows `ConfigOptVirtualBool`: two named boolean options, `soundEnabled` and
`ledDisabled`, packed into individual bits of a single `flags` value instead
of each needing their own EEPROM storage.

`ledDisabled` also demonstrates inverted polarity -- it reads `true` when its
bit is *clear*, so the option can default to a sensible value even though a
freshly-reset packed word is all zero.

Build for a Huzzah (ESP8266) board:

```sh
pio run -e huzzah
```
