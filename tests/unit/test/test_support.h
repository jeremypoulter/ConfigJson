#ifndef CONFIGJSON_TEST_SUPPORT_H
#define CONFIGJSON_TEST_SUPPORT_H

#include <ConfigOpt.h>

// ConfigJson itself picks its JSON document type via CONFIG_JSON_DOC
// (JsonDocument on ArduinoJson v7, DynamicJsonDocument on v6), so the tests
// build against both majors too. Only the constructor differs -- v6's
// DynamicJsonDocument needs an explicit capacity -- so wrap it in a macro
// rather than hardcoding the v7-only default-constructible JsonDocument.
#if ARDUINOJSON_VERSION_MAJOR >= 7
#define TEST_JSON_DOC(name) CONFIG_JSON_DOC name
#else
#define TEST_JSON_DOC(name) CONFIG_JSON_DOC name(1024)
#endif

#endif
