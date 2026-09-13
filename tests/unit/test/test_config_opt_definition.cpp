#include <unity.h>

#include <ArduinoJson.h>
#include <ConfigOptDefinition.h>

#include "test_support.h"

static void test_get_returns_the_bound_value() {
  int32_t backing = 42;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");

  TEST_ASSERT_EQUAL(42, opt.get());
}

static void test_name_returns_long_or_short_form() {
  int32_t backing = 0;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");

  TEST_ASSERT_EQUAL_STRING("number", opt.name(true));
  TEST_ASSERT_EQUAL_STRING("n", opt.name(false));
}

static void test_set_reports_whether_the_value_changed() {
  int32_t backing = 5;
  ConfigOptDefinition<int32_t> opt(backing, 5, "number", "n");

  TEST_ASSERT_FALSE(opt.set(5));
  TEST_ASSERT_TRUE(opt.set(6));
  TEST_ASSERT_EQUAL(6, backing);
}

static void test_set_default_resets_the_backing_value() {
  int32_t backing = 99;
  ConfigOptDefinition<int32_t> opt(backing, 7, "number", "n");

  opt.setDefault();

  TEST_ASSERT_EQUAL(7, backing);
}

static void test_serialize_uses_requested_name_form() {
  int32_t backing = 3;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");
  TEST_JSON_DOC(longDoc);
  TEST_JSON_DOC(shortDoc);

  TEST_ASSERT_TRUE(opt.serialize(longDoc, true, false, false));
  TEST_ASSERT_EQUAL(3, longDoc["number"].as<int32_t>());

  TEST_ASSERT_TRUE(opt.serialize(shortDoc, false, false, false));
  TEST_ASSERT_EQUAL(3, shortDoc["n"].as<int32_t>());
}

static void test_serialize_compact_output_skips_default_values() {
  int32_t backing = 0;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");
  TEST_JSON_DOC(doc);

  TEST_ASSERT_FALSE(opt.serialize(doc, true, true, false));
  TEST_ASSERT_TRUE(doc["number"].isNull());

  backing = 1;
  TEST_ASSERT_TRUE(opt.serialize(doc, true, true, false));
  TEST_ASSERT_EQUAL(1, doc["number"].as<int32_t>());
}

static void test_deserialize_prefers_long_name_over_short() {
  int32_t backing = 0;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");
  TEST_JSON_DOC(doc);
  doc["number"] = 10;
  doc["n"] = 20;

  TEST_ASSERT_TRUE(opt.deserialize(doc));
  TEST_ASSERT_EQUAL(10, backing);
}

static void test_deserialize_falls_back_to_short_name() {
  int32_t backing = 0;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");
  TEST_JSON_DOC(doc);
  doc["n"] = 20;

  TEST_ASSERT_TRUE(opt.deserialize(doc));
  TEST_ASSERT_EQUAL(20, backing);
}

static void test_deserialize_returns_false_when_key_absent() {
  int32_t backing = 4;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");
  TEST_JSON_DOC(doc);
  doc["other"] = 1;

  TEST_ASSERT_FALSE(opt.deserialize(doc));
  TEST_ASSERT_EQUAL(4, backing);
}

static void test_deserialize_returns_false_when_value_unchanged() {
  int32_t backing = 5;
  ConfigOptDefinition<int32_t> opt(backing, 0, "number", "n");
  TEST_JSON_DOC(doc);
  doc["number"] = 5;

  TEST_ASSERT_FALSE(opt.deserialize(doc));
}

static void test_bool_option_round_trips() {
  bool backing = false;
  ConfigOptDefinition<bool> opt(backing, false, "truth", "t");
  TEST_JSON_DOC(doc);
  doc["truth"] = true;

  TEST_ASSERT_TRUE(opt.deserialize(doc));
  TEST_ASSERT_TRUE(backing);
}

static void test_string_option_round_trips() {
  String backing = "";
  ConfigOptDefinition<String> opt(backing, "", "name", "nm");
  TEST_JSON_DOC(doc);
  doc["name"] = "hello";

  TEST_ASSERT_TRUE(opt.deserialize(doc));
  TEST_ASSERT_TRUE(backing.equals("hello"));

  TEST_JSON_DOC(out);
  TEST_ASSERT_TRUE(opt.serialize(out, true, false, false));
  TEST_ASSERT_EQUAL_STRING("hello", out["name"].as<const char *>());
}

void runConfigOptDefinitionTests() {
  RUN_TEST(test_get_returns_the_bound_value);
  RUN_TEST(test_name_returns_long_or_short_form);
  RUN_TEST(test_set_reports_whether_the_value_changed);
  RUN_TEST(test_set_default_resets_the_backing_value);
  RUN_TEST(test_serialize_uses_requested_name_form);
  RUN_TEST(test_serialize_compact_output_skips_default_values);
  RUN_TEST(test_deserialize_prefers_long_name_over_short);
  RUN_TEST(test_deserialize_falls_back_to_short_name);
  RUN_TEST(test_deserialize_returns_false_when_key_absent);
  RUN_TEST(test_deserialize_returns_false_when_value_unchanged);
  RUN_TEST(test_bool_option_round_trips);
  RUN_TEST(test_string_option_round_trips);
}
