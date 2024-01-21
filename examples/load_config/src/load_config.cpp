#include <Arduino.h>
#include <ConfigJson.h>

#define EEPROM_SIZE     512

String string;
String secret;
int32_t number;
bool truth;

ConfigOpt *opts[] = 
{
  new ConfigOptDefinition <String>(string, "string", "string", "s"),
  new ConfigOptDefinition <int32_t>(number, 0, "number", "n"),
  new ConfigOptDefinition <bool>(truth, false, "truth", "t"),
  new ConfigOptSecret(secret, "secret", "password", "p")
};

ConfigJson config(opts, sizeof(opts) / sizeof(opts[0]), EEPROM_SIZE);

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ConfigJson");
  Serial.println("");
  Serial.println("");

  if(!config.load()) {
    Serial.print("No JSON config found");
  }

  Serial.println("Long names");
  config.serialize(Serial, true);
  Serial.println("");
  Serial.println("");

  Serial.println("Short names");
  config.serialize(Serial, false);
  Serial.println("");
  Serial.println("");

  Serial.println("Compact output");
  config.serialize(Serial, false, true, false);
  Serial.println("");
  Serial.println("");

  Serial.println("Hide secrets");
  config.serialize(Serial, true, false, true);
  Serial.println("");
}

void loop()
{
}
