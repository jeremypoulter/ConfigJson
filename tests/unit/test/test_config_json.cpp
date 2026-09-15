#include <unity.h>

#include <EEPROM.h>
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

static void test_deserialize_updates_only_the_named_options_and_reports_changes() {
  Fixture fx;
  String changed[4];
  size_t changedCount = 0;
  fx.config.onChanged([&](String opt) {
    changed[changedCount++] = opt;
  });

  TEST_ASSERT_TRUE(fx.config.deserialize("{\"name\":\"widget\",\"number\":5}"));

  TEST_ASSERT_TRUE(fx.name.equals("widget"));
  TEST_ASSERT_EQUAL(5, fx.number);
  TEST_ASSERT_FALSE(fx.truth);
  TEST_ASSERT_EQUAL(2, changedCount);
  TEST_ASSERT_TRUE(changed[0].equals("name"));
  TEST_ASSERT_TRUE(changed[1].equals("number"));
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
  EEPROM.wipe();
  Fixture fx;

  TEST_ASSERT_FALSE(fx.config.load());
  TEST_ASSERT_TRUE(fx.name.equals("default"));
}

static void test_load_with_merge_skips_the_implicit_reset() {
  EEPROM.wipe();
  Fixture fx;
  fx.name = "untouched";

  TEST_ASSERT_FALSE(fx.config.load(true));
  TEST_ASSERT_TRUE(fx.name.equals("untouched"));
}

static void test_commit_persists_to_eeprom_for_a_later_load() {
  EEPROM.wipe();
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
}
