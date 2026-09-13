#include <unity.h>

#include <ConfigOptVirtualMaskedBool.h>

static void test_set_marks_the_bit_dirty_in_the_change_mask() {
  uint32_t rawBacking = 0;
  uint32_t changeBacking = 0;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptDefinition<uint32_t> change(changeBacking, 0, "changed", "c");
  ConfigOptVirtualMaskedBool flag(raw, change, 0x01, 0x01, "enabled", "e");

  TEST_ASSERT_TRUE(flag.set(true));
  TEST_ASSERT_EQUAL_HEX32(0x01, rawBacking);
  TEST_ASSERT_EQUAL_HEX32(0x01, changeBacking);
}

static void test_set_does_not_touch_the_change_mask_when_value_is_unchanged() {
  uint32_t rawBacking = 0x01;
  uint32_t changeBacking = 0;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptDefinition<uint32_t> change(changeBacking, 0, "changed", "c");
  ConfigOptVirtualMaskedBool flag(raw, change, 0x01, 0x01, "enabled", "e");

  TEST_ASSERT_FALSE(flag.set(true));
  TEST_ASSERT_EQUAL_HEX32(0, changeBacking);
}

static void test_change_mask_only_tracks_bits_that_were_actually_set() {
  uint32_t rawBacking = 0;
  uint32_t changeBacking = 0;
  ConfigOptDefinition<uint32_t> raw(rawBacking, 0, "flags", "f");
  ConfigOptDefinition<uint32_t> change(changeBacking, 0, "changed", "c");
  ConfigOptVirtualMaskedBool bitA(raw, change, 0x01, 0x01, "a", "a");
  ConfigOptVirtualMaskedBool bitB(raw, change, 0x02, 0x02, "b", "b");

  bitA.set(true);

  TEST_ASSERT_EQUAL_HEX32(0x01, changeBacking);
  TEST_ASSERT_FALSE(bitB.get());
}

void runConfigOptVirtualMaskedBoolTests() {
  RUN_TEST(test_set_marks_the_bit_dirty_in_the_change_mask);
  RUN_TEST(test_set_does_not_touch_the_change_mask_when_value_is_unchanged);
  RUN_TEST(test_change_mask_only_tracks_bits_that_were_actually_set);
}
