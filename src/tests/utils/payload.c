#include <axium/utils/payload.h>
#include <test_common.h>

void test_patch_basic(void) {
  char buf[] = "Hello MARKER World";
  patch((uint8_t *)buf, strlen(buf), "MARKER", 6, "AXIUM!", 6);
  ASSERT_STREQ(buf, "Hello AXIUM! World", "Basic patch works");
}

void test_payload_push_str(void) {
  payload_t p;
  payload_init(&p);
  payload_push_str(&p, "AXIUM");
  ASSERT_EQ(p.size, 5, "payload_push_str size correct");
  ASSERT_TRUE(memcmp(p.data, "AXIUM", 5) == 0,
              "payload_push_str content correct");
  payload_fini(&p);
}

void test_payload_pack_designated(void) {
  payload_t p;
  payload_init(&p);
  // Designated initializers for sparse chain construction
  PAYLOAD_PACK(&p, uint64_t, [0] = 0x1111, [3] = 0x3333);
  ASSERT_EQ(p.size, 4 * 8, "Sparse PACK size correct");
  uint64_t *d = (uint64_t *)p.data;
  ASSERT_EQ(d[0], 0x1111, "Item 0 correct");
  ASSERT_EQ(d[1], 0, "Item 1 zero-filled");
  ASSERT_EQ(d[3], 0x3333, "Item 3 correct");
  payload_fini(&p);
}

void test_payload_fill_to_variants(void) {
  payload_t p;
  payload_init(&p);

  // 1. Fill to offset with zeros (filler=NULL)
  payload_fill_to(&p, 16, NULL, 0);
  ASSERT_EQ(p.size, 16, "fill_to zero size correct");
  for (size_t i = 0; i < 16; i++)
    ASSERT_EQ(p.data[i], 0, "Filled with zeros");

  // 2. Single-byte optimized fill (filler_size=1)
  uint8_t f = 0x41;
  payload_fill_to(&p, 32, &f, 1);
  ASSERT_EQ(p.size, 32, "fill_to single byte size correct");
  for (size_t i = 16; i < 32; i++)
    ASSERT_EQ(p.data[i], 0x41, "Filled with 'A'");

  // 3. Multi-byte pattern fill
  payload_fill_to(&p, 38, "ABC", 3);
  // current 32. need 6 more. ABCABC
  ASSERT_TRUE(memcmp(p.data + 32, "ABCABC", 6) == 0, "Pattern fill correct");

  payload_fini(&p);
}

void test_payload_at_helpers(void) {
  payload_t p;
  payload_init(&p);

  payload_at_str(&p, 0x10, "MARK");
  payload_at_u64(&p, 0x20, 0x1337);

  ASSERT_EQ(p.size, 0x20 + 8, "payload_at helpers size correct");
  ASSERT_TRUE(memcmp(p.data + 0x10, "MARK", 4) == 0, "at_str correct");
  ASSERT_EQ(*(uint64_t *)(p.data + 0x20), 0x1337, "at_u64 correct");
  ASSERT_EQ(p.data[5], 0, "Gap is zero-filled");

  payload_fini(&p);
}

void test_payload_rel_patching_all(void) {
  payload_t p;
  payload_init(&p);

  // rel8
  payload_at_u8(&p, 0, 0xAA);
  payload_patch_rel8(&p, 0xAA, 11); // target 11, cur 0+1=1. disp 10
  ASSERT_EQ(*(uint8_t *)p.data, 10, "rel8 correct");

  // rel16
  payload_at_u16(&p, 10, 0xBBBB);
  payload_patch_rel16(&p, 0xBBBB, 32); // target 32, cur 10+2=12. disp 20
  ASSERT_EQ(*(uint16_t *)(p.data + 10), 20, "rel16 correct");

  // rel32
  payload_at_u32(&p, 20, 0xCCCCCCCC);
  payload_patch_rel32(&p, 0xCCCCCCCC, 54); // target 54, cur 20+4=24. disp 30
  ASSERT_EQ(*(uint32_t *)(p.data + 20), 30, "rel32 correct");

  // rel64
  payload_at_u64(&p, 40, 0xDDDDDDDD);
  payload_patch_rel64(&p, 0xDDDDDDDD, 148); // target 148, cur 40+8=48. disp 100
  ASSERT_EQ(*(uint64_t *)(p.data + 40), 100, "rel64 correct");

  payload_fini(&p);
}

void test_payload_large_expansion(void) {
  payload_t p;
  payload_init(&p);

  // Test ensure_capacity with a very large jump
  payload_fill_to(&p, 10000, "X", 1);
  ASSERT_EQ(p.size, 10000, "Large expansion size correct");
  ASSERT_TRUE(p.capacity >= 10000, "Capacity sufficiently expanded");
  ASSERT_EQ(p.data[9999], 'X', "Last byte correct");

  payload_fini(&p);
}

int main(void) {
  test_patch_basic();
  test_payload_push_str();
  test_payload_pack_designated();
  test_payload_fill_to_variants();
  test_payload_at_helpers();
  test_payload_rel_patching_all();
  test_payload_large_expansion();
  return 0;
}
