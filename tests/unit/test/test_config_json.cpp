#include <unity.h>

#include <ConfigJson.h>

#include "test_support.h"

namespace {

// A small, representative set of options -- one of each ConfigOpt flavour
// ConfigJson is expected to drive -- wired up the same way a real sketch
// would in its ConfigOpt* array.
struct Fixture {
  String name;
  int32_t number;
  bool truth;
  String secret;

  ConfigOpt *opts[4];
  ConfigJson config;

  Fixture() :
    name(""),
    number(0),
    truth(false),
    secret(""),
    opts{
      new ConfigOptDefinition<String>(name, "default", "name", "nm"),
      new ConfigOptDefinition<int32_t>(number, 1, "number", "n"),
      new ConfigOptDefinition<bool>(truth, false, "truth", "t"),
      new ConfigOptSecret(secret, "", "password", "p")
    },
    config(opts, 4, 512)
  {
  }

  ~Fixture() {
    for (size_t i = 0; i < 4; i++) {
      delete opts[i];
    }
  }
};

}  // namespace

// A trivial Print sink so serialize(Print&, ...) can be exercised without a
// real Serial/hardware stream.
class StringPrint : public Print {
  public:
    String buffer;

    size_t write(uint8_t c) override {
      buffer += (char) c;
      return 1;
    }
};

static void test_serialize_includes_every_option() {
  Fixture fx;
  fx.name = "widget";
  fx.number = 5;
  fx.truth = true;
  fx.secret = "hunter2";

  String json;
  TEST_ASSERT_TRUE(fx.config.serialize(json, true, false, false));

  TEST_JSON_DOC(doc);
  TEST_ASSERT_FALSE(deserializeJson(doc, json));
  TEST_ASSERT_EQUAL_STRING("widget", doc["name"].as<const char *>());
  TEST_ASSERT_EQUAL(5, doc["number"].as<int32_t>());
  TEST_ASSERT_TRUE(doc["truth"].as<bool>());
  TEST_ASSERT_EQUAL_STRING("hunter2", doc["password"].as<const char *>());
}

static void test_serialize_to_a_print_stream_matches_serialize_to_string() {
  Fixture fx;
  fx.name = "widget";
  fx.number = 5;

  String expected;
  TEST_ASSERT_TRUE(fx.config.serialize(expected, true, false, false));

  StringPrint out;
  TEST_ASSERT_TRUE(fx.config.serialize(out, true, false, false));
  TEST_ASSERT_TRUE(out.buffer.equals(expected));
}

// A handler is a plain function, so it records into globals.
static String changedNames[4];
static size_t changedCount = 0;
static void recordChange(String opt) {
  changedNames[changedCount++] = opt;
}

static void test_deserialize_updates_only_the_named_options_and_reports_changes() {
  Fixture fx;
  changedCount = 0;
  fx.config.onChanged(recordChange);

  TEST_ASSERT_TRUE(fx.config.deserialize("{\"name\":\"widget\",\"number\":5}"));

  TEST_ASSERT_TRUE(fx.name.equals("widget"));
  TEST_ASSERT_EQUAL(5, fx.number);
  TEST_ASSERT_FALSE(fx.truth);
  TEST_ASSERT_EQUAL(2, changedCount);
  TEST_ASSERT_TRUE(changedNames[0].equals("name"));
  TEST_ASSERT_TRUE(changedNames[1].equals("number"));
}

static void test_deserialize_returns_false_for_invalid_json() {
  Fixture fx;

  TEST_ASSERT_FALSE(fx.config.deserialize("not json"));
}

static void test_set_convenience_overloads_update_the_named_option() {
  Fixture fx;

  TEST_ASSERT_TRUE(fx.config.set("truth", true));
  TEST_ASSERT_TRUE(fx.truth);

  TEST_ASSERT_TRUE(fx.config.set(String("number"), 42));
  TEST_ASSERT_EQUAL(42, fx.number);
}

static void test_reset_restores_every_option_to_its_default() {
  Fixture fx;
  fx.name = "changed";
  fx.number = 99;
  fx.truth = true;
  fx.secret = "hunter2";

  fx.config.reset();

  TEST_ASSERT_TRUE(fx.name.equals("default"));
  TEST_ASSERT_EQUAL(1, fx.number);
  TEST_ASSERT_FALSE(fx.truth);
  TEST_ASSERT_TRUE(fx.secret.equals(""));
}

static void test_load_returns_false_and_leaves_defaults_when_eeprom_is_blank() {
  wipeEeprom();
  Fixture fx;

  TEST_ASSERT_FALSE(fx.config.load());
  TEST_ASSERT_TRUE(fx.name.equals("default"));
}

static void test_load_with_merge_skips_the_implicit_reset() {
  wipeEeprom();
  Fixture fx;
  fx.name = "untouched";

  TEST_ASSERT_FALSE(fx.config.load(true));
  TEST_ASSERT_TRUE(fx.name.equals("untouched"));
}

static void test_commit_persists_to_eeprom_for_a_later_load() {
  wipeEeprom();
  {
    Fixture writer;
    writer.config.reset();
    writer.config.set("name", String("widget"));
    writer.config.set("number", 7);
    writer.config.commit();
  }

  Fixture reader;
  TEST_ASSERT_TRUE(reader.config.load());
  TEST_ASSERT_TRUE(reader.name.equals("widget"));
  TEST_ASSERT_EQUAL(7, reader.number);
}

static void test_commit_refuses_a_record_over_its_limit_and_keeps_the_stored_one() {
  wipeEeprom();
  Fixture fx;
  ConfigJson config(fx.opts, 4, 512, 0, 64);
  config.set("name", String("widget"));
  TEST_ASSERT_TRUE(config.commit());

  String big;
  for (int i = 0; i < 100; i++) {
    big += 'x';
  }
  config.set("name", big);
  TEST_ASSERT_FALSE(config.commit());

  Fixture reader;
  TEST_ASSERT_TRUE(reader.config.load());
  TEST_ASSERT_TRUE(reader.name.equals("widget"));
}

static void test_record_size_is_clamped_to_the_storage_it_opens() {
  wipeEeprom();
  Fixture fx;
  // Asks for a record far larger than the 512-byte storage. It is clamped, so a
  // value that only fits the larger record is refused, not written past the end.
  ConfigJson config(fx.opts, 4, TEST_STORAGE_SIZE, 0, 4096);
  String big;
  for (int i = 0; i < 600; i++) {
    big += 'x';
  }
  // Assigned directly: this test is about commit(), not set().
  config.reset();
  fx.name = big;
  TEST_ASSERT_FALSE(config.commit());
}

// Writes a stored record with the given length header over the given bytes.
static void writeRecord(const String &json, size_t length) {
  const uint8_t header[] = {(uint8_t)(length & 0xff), (uint8_t)(length >> 8)};
  pokeEeprom(0, header, sizeof(header));
  pokeEeprom(2, (const uint8_t *) json.c_str(), json.length());
}

// A JSON object padded with spaces to exactly `length` bytes.
static String paddedRecord(size_t length) {
  String json = "{\"name\":\"widget\"}";
  while (json.length() < length) {
    json += ' ';
  }
  return json;
}

static void test_set_stores_a_long_string_in_full() {
  wipeEeprom();
  Fixture fx;
  // Longer than the fixed scratch document ArduinoJson v6 used to get, which
  // cut the value short and still reported success.
  String big;
  for (int i = 0; i < 300; i++) {
    big += 'x';
  }
  TEST_ASSERT_TRUE(fx.config.set("name", big));
  TEST_ASSERT_TRUE(fx.name.equals(big));
}

static void test_load_returns_false_for_a_damaged_record_and_leaves_defaults() {
  wipeEeprom();
  Fixture fx;
  // A header load() accepts, over JSON that doesn't parse
  const uint8_t damaged[] = {5, 0, '{', 'x', 'x', 'x', 'x'};
  pokeEeprom(0, damaged, sizeof(damaged));

  TEST_ASSERT_FALSE(fx.config.load());
  TEST_ASSERT_TRUE(fx.name.equals("default"));
}

static void test_load_refuses_a_record_that_runs_past_the_storage() {
  // A record ending on the storage's last byte is read.
  wipeEeprom();
  Fixture fits;
  writeRecord(paddedRecord(TEST_STORAGE_SIZE - 2), TEST_STORAGE_SIZE - 2);
  TEST_ASSERT_TRUE(fits.config.load());
  TEST_ASSERT_TRUE(fits.name.equals("widget"));

  // One byte longer runs one byte past the storage, so it is refused. Its
  // last byte is valid JSON, so only the bounds check can refuse it.
  wipeEeprom();
  Fixture runsOver;
  writeRecord(paddedRecord(TEST_STORAGE_SIZE - 1), TEST_STORAGE_SIZE - 1);
  TEST_ASSERT_FALSE(runsOver.config.load());
  TEST_ASSERT_TRUE(runsOver.name.equals("default"));
}

static void test_stored_unreadable_tells_a_damaged_record_from_blank_and_valid() {
  wipeEeprom();
  Fixture fx;
  TEST_ASSERT_FALSE(fx.config.storedUnreadable());

  // A header load() accepts, over JSON that doesn't parse
  const uint8_t damaged[] = {5, 0, '{', 'x', 'x', 'x', 'x'};
  pokeEeprom(0, damaged, sizeof(damaged));
  TEST_ASSERT_TRUE(fx.config.storedUnreadable());

  fx.config.reset();
  TEST_ASSERT_TRUE(fx.config.commit());
  TEST_ASSERT_FALSE(fx.config.storedUnreadable());
}

// The limit is the 16-bit length header's maximum plus its own two bytes. A
// storage that can't be reached on the emulated EEPROMs is tested through the
// helper that computes it.
static void test_record_limit_is_exactly_what_the_length_header_can_describe() {
  const size_t header = 2;
  const size_t maxRecord = 0xFFFF + header;
  // Storage larger than the header can describe: the default is clamped to it
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(100000, 0, 0) == maxRecord);
  // A payload of exactly 65535 bytes, plus the header, is accepted
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(100000, 0, maxRecord) == maxRecord);
  // One byte more is clamped, not allowed to wrap or to be written
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(100000, 0, maxRecord + 1) == maxRecord);
  // Smaller storage: the rest after the offset, with an explicit size clamped to it
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(512, 0, 0) == 512);
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(512, 100, 0) == 412);
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(512, 0, 1000) == 512);
  // An offset past the end leaves nothing, rather than underflowing
  TEST_ASSERT_TRUE(ConfigJson::recordLimit(512, 600, 0) == 0);
}

void runConfigJsonTests() {
  RUN_TEST(test_serialize_includes_every_option);
  RUN_TEST(test_serialize_to_a_print_stream_matches_serialize_to_string);
  RUN_TEST(test_deserialize_updates_only_the_named_options_and_reports_changes);
  RUN_TEST(test_deserialize_returns_false_for_invalid_json);
  RUN_TEST(test_set_convenience_overloads_update_the_named_option);
  RUN_TEST(test_reset_restores_every_option_to_its_default);
  RUN_TEST(test_load_returns_false_and_leaves_defaults_when_eeprom_is_blank);
  RUN_TEST(test_load_with_merge_skips_the_implicit_reset);
  RUN_TEST(test_commit_persists_to_eeprom_for_a_later_load);
  RUN_TEST(test_commit_refuses_a_record_over_its_limit_and_keeps_the_stored_one);
  RUN_TEST(test_record_size_is_clamped_to_the_storage_it_opens);
  RUN_TEST(test_record_limit_is_exactly_what_the_length_header_can_describe);
  RUN_TEST(test_set_stores_a_long_string_in_full);
  RUN_TEST(test_load_returns_false_for_a_damaged_record_and_leaves_defaults);
  RUN_TEST(test_load_refuses_a_record_that_runs_past_the_storage);
  RUN_TEST(test_stored_unreadable_tells_a_damaged_record_from_blank_and_valid);
}
