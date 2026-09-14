// Demonstrates ConfigOptVirtualBool: packing several named boolean options
// into the bits of a single stored uint32_t, so each flag doesn't need its
// own EEPROM slot.
#include <Arduino.h>
#include <ConfigJson.h>

#define EEPROM_SIZE 512

String deviceName;
uint32_t flags = 0;

ConfigOptDefinition<uint32_t> flagsOpt(flags, 0, "flags", "f");

ConfigOpt *opts[] =
{
  new ConfigOptDefinition<String>(deviceName, "device", "name", "n"),
  &flagsOpt,
  // soundEnabled reads true when bit 0 is set.
  new ConfigOptVirtualBool(flagsOpt, 0x01, 0x01, "soundEnabled", "se"),
  // ledDisabled reads true when bit 1 is *clear* -- inverted polarity, for a
  // flag that should default to "on" even though the packed word defaults
  // to all-zero.
  new ConfigOptVirtualBool(flagsOpt, 0x02, 0x00, "ledDisabled", "ld"),
};

ConfigJson config(opts, sizeof(opts) / sizeof(opts[0]), EEPROM_SIZE);

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("virtual_bool_flags");

  if (!config.load()) {
    Serial.println("No stored config found, using defaults");
  }

  Serial.println("Current config:");
  config.serialize(Serial);
  Serial.println("");

  // Individual flags can be changed by name...
  config.set("soundEnabled", true);
  // ...and the change is visible both on the virtual flag and the packed
  // word it lives in.
  Serial.print("flags after enabling sound: 0b");
  Serial.println(flags, BIN);

  config.commit();
}

void loop()
{
}
