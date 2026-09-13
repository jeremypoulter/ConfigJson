#include <unity.h>

#include <ArduinoJson.h>
#include <ConfigOptSecret.h>

static void test_set_ignores_the_dummy_password_sentinel() {
  String backing = "original";
  ConfigOptSecret opt(backing, "", "password", "p");

  TEST_ASSERT_FALSE(opt.set(String(ConfigOptSecret::DUMMY_PASSWORD)));
  TEST_ASSERT_TRUE(backing.equals("original"));
}

static void test_set_accepts_a_real_value() {
  String backing = "";
  ConfigOptSecret opt(backing, "", "password", "p");

  TEST_ASSERT_TRUE(opt.set(String("hunter2")));
  TEST_ASSERT_TRUE(backing.equals("hunter2"));
}

static void test_serialize_reveals_the_secret_when_not_hidden() {
  String backing = "hunter2";
  ConfigOptSecret opt(backing, "", "password", "p");
  JsonDocument doc;

  TEST_ASSERT_TRUE(opt.serialize(doc, true, false, false));
  TEST_ASSERT_EQUAL_STRING("hunter2", doc["password"].as<const char *>());
}

static void test_serialize_masks_a_set_secret_when_hidden() {
  String backing = "hunter2";
  ConfigOptSecret opt(backing, "", "password", "p");
  JsonDocument doc;

  TEST_ASSERT_TRUE(opt.serialize(doc, true, false, true));
  TEST_ASSERT_EQUAL_STRING("_DUMMY_PASSWORD", doc["password"].as<const char *>());
}

static void test_serialize_leaves_an_unset_secret_blank_when_hidden() {
  String backing = "";
  ConfigOptSecret opt(backing, "", "password", "p");
  JsonDocument doc;

  TEST_ASSERT_TRUE(opt.serialize(doc, true, false, true));
  TEST_ASSERT_EQUAL_STRING("", doc["password"].as<const char *>());
}

static void test_serialize_compact_output_skips_default_secret() {
  String backing = "";
  ConfigOptSecret opt(backing, "", "password", "p");
  JsonDocument doc;

  TEST_ASSERT_FALSE(opt.serialize(doc, true, true, false));
  TEST_ASSERT_TRUE(doc["password"].isNull());
}

void runConfigOptSecretTests() {
  RUN_TEST(test_set_ignores_the_dummy_password_sentinel);
  RUN_TEST(test_set_accepts_a_real_value);
  RUN_TEST(test_serialize_reveals_the_secret_when_not_hidden);
  RUN_TEST(test_serialize_masks_a_set_secret_when_hidden);
  RUN_TEST(test_serialize_leaves_an_unset_secret_blank_when_hidden);
  RUN_TEST(test_serialize_compact_output_skips_default_secret);
}
