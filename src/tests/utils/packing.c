#include <axium/utils/packing.h>
#include <test_common.h>

void test_packing_le(void) {
  ASSERT_EQ(p16le(0x1122), 0x1122,
            "p16le on little-endian host (manual check if needed)");
  ASSERT_EQ(p32le(0x11223344), 0x11223344, "p32le correct");
  ASSERT_EQ(p64le(0x1122334455667788), 0x1122334455667788, "p64le correct");
}

void test_packing_be(void) {
  // We check if the swapped value matches what we expect
  uint32_t val = 0x11223344;
  uint32_t swapped = p32be(val);

  // On a little-endian host, p32be(0x11223344) should be 0x44332211
  // We use the helper to verify
  ASSERT_EQ(swapped, __builtin_bswap32(val), "p32be performs swap on LE host");
}

void test_unpacking(void) {
  uint8_t data[] = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

  ASSERT_EQ(u16le(data), 0x6677, "u16le correct");
  ASSERT_EQ(u32le(data), 0x44556677, "u32le correct");
  ASSERT_EQ(u64le(data), 0x0011223344556677, "u64le correct");

  // Test big-endian unpacking
  ASSERT_EQ(u32be(data), 0x77665544, "u32be correct");
}

int main(void) {
  test_packing_le();
  test_packing_be();
  test_unpacking();
  return 0;
}
