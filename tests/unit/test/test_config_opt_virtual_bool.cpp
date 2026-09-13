#include <unity.h>

#include <ArduinoJson.h>
#include <ConfigOptVirtualBool.h>

// Named "flag" rather than "bit" -- Arduino.h #defines bit(b) as a function
// macro, which swallows a same-named local variable's constructor arguments.
static void test_get_reads_true_when_bit_matches_expected() {
  uint32_t rawBacking = 0;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");

  TEST_ASSERT_FALSE(flag.get());
  raw.set(0x01);
  TEST_ASSERT_TRUE(flag.get());
}

static void test_get_supports_inverted_polarity() {
  // expected == 0 means the virtual flag reads true when the bit is clear.
  uint32_t rawBacking = 0x01;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x00, "enabled", "e");

  TEST_ASSERT_FALSE(flag.get());
  raw.set(0x00);
  TEST_ASSERT_TRUE(flag.get());
}

static void test_set_true_sets_only_its_own_bit() {
  uint32_t rawBacking = 0x02;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");

  TEST_ASSERT_TRUE(flag.set(true));
  TEST_ASSERT_EQUAL_HEX32(0x03, rawBacking);
}

static void test_set_false_clears_only_its_own_bit() {
  uint32_t rawBacking = 0x03;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");

  TEST_ASSERT_TRUE(flag.set(false));
  TEST_ASSERT_EQUAL_HEX32(0x02, rawBacking);
}

static void test_set_returns_false_when_the_bit_was_already_correct() {
  uint32_t rawBacking = 0x01;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");

  TEST_ASSERT_FALSE(flag.set(true));
}

static void test_serialize_writes_current_value_unless_compact() {
  uint32_t rawBacking = 0x01;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");
  JsonDocument doc;

  TEST_ASSERT_TRUE(flag.serialize(doc, true, false, false));
  TEST_ASSERT_TRUE(doc["enabled"].as<bool>());

  JsonDocument compactDoc;
  TEST_ASSERT_FALSE(flag.serialize(compactDoc, true, true, false));
  TEST_ASSERT_TRUE(compactDoc["enabled"].isNull());
}

static void test_deserialize_sets_the_virtual_bit_from_either_name() {
  uint32_t rawBacking = 0;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");
  JsonDocument doc;
  doc["e"] = true;

  TEST_ASSERT_TRUE(flag.deserialize(doc));
  TEST_ASSERT_EQUAL_HEX32(0x01, rawBacking);
}

static void test_set_default_is_a_no_op() {
  uint32_t rawBacking = 0x01;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptVirtualBool flag(raw, 0x01, 0x01, "enabled", "e");

  flag.setDefault();

  TEST_ASSERT_EQUAL_HEX32(0x01, rawBacking);
}

void runConfigOptVirtualBoolTests() {
  RUN_TEST(test_get_reads_true_when_bit_matches_expected);
  RUN_TEST(test_get_supports_inverted_polarity);
  RUN_TEST(test_set_true_sets_only_its_own_bit);
  RUN_TEST(test_set_false_clears_only_its_own_bit);
  RUN_TEST(test_set_returns_false_when_the_bit_was_already_correct);
  RUN_TEST(test_serialize_writes_current_value_unless_compact);
  RUN_TEST(test_deserialize_sets_the_virtual_bit_from_either_name);
  RUN_TEST(test_set_default_is_a_no_op);
}
