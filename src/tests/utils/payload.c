#include <axium/utils/payload.h>
#include <test_common.h>

void test_patch_basic(void) {
  char buf[] = "Hello MARKER World";
  patch((uint8_t *)buf, strlen(buf), "MARKER", 6, "AXIUM!", 6);
  ASSERT_STREQ(buf, "Hello AXIUM! World", "Basic patch works");
}

void test_patch_edge_cases(void) {
  char buf[] = "Test";
  // Buffer smaller than marker
  patch((uint8_t *)buf, 4, "LONGMARKER", 10, "X", 1);
  ASSERT_STREQ(buf, "Test", "Patch does nothing if buffer < marker");

  // Zero replacement size (should zero-fill the marker)
  char buf2[] = "AAABBBCCC";
  patch((uint8_t *)buf2, 9, "BBB", 3, NULL, 0);
  ASSERT_TRUE(memcmp(buf2, "AAA\0\0\0CCC", 9) == 0,
              "Zero-size replacement zero-fills the marker");
}

void test_payload_push_ints(void) {
  payload_t p;
  payload_init(&p);

  payload_push_u8(&p, 0x11);
  payload_push_u16(&p, 0x2233);
  payload_push_u32(&p, 0x44556677);
  payload_push_u64(&p, 0x8899AABBCCDDEEFF);

  ASSERT_EQ(p.size, 1 + 2 + 4 + 8, "Payload size correct after integer pushes");
  ASSERT_EQ(*(uint8_t *)(p.data + 0), 0x11, "u8 push correct");
  ASSERT_EQ(*(uint16_t *)(p.data + 1), 0x2233, "u16 push correct");
  ASSERT_EQ(*(uint32_t *)(p.data + 3), 0x44556677, "u32 push correct");
  ASSERT_EQ(*(uint64_t *)(p.data + 7), 0x8899AABBCCDDEEFF, "u64 push correct");

  payload_fini(&p);
}

void test_payload_patch_ints(void) {
  payload_t p;
  payload_init(&p);

  payload_push_u8(&p, 0xAA);
  payload_push_u16(&p, 0xBBBB);
  payload_push_u32(&p, 0xCCCCCCCC);
  payload_push_u64(&p, 0xDDDDDDDDDDDDDDDD);

  payload_patch_u8(&p, 0xAA, 0x11);
  payload_patch_u16(&p, 0xBBBB, 0x2222);
  payload_patch_u32(&p, 0xCCCCCCCC, 0x33333333);
  payload_patch_u64(&p, 0xDDDDDDDDDDDDDDDD, 0x4444444444444444);

  ASSERT_EQ(*(uint8_t *)(p.data + 0), 0x11, "u8 patch correct");
  ASSERT_EQ(*(uint16_t *)(p.data + 1), 0x2222, "u16 patch correct");
  ASSERT_EQ(*(uint32_t *)(p.data + 3), 0x33333333, "u32 patch correct");
  ASSERT_EQ(*(uint64_t *)(p.data + 7), 0x4444444444444444, "u64 patch correct");

  payload_fini(&p);
}

void test_payload_rel_all_sizes(void) {
  payload_t p;
  payload_init(&p);

  // 8-bit rel
  size_t off8 = p.size;
  payload_push_u8(&p, 0xFF);
  payload_patch_rel8(&p, 0xFF, off8 + 1 + 10); // Target is 10 bytes ahead
  ASSERT_EQ(*(uint8_t *)(p.data + off8), 10, "rel8 patch correct");

  // 16-bit rel
  size_t off16 = p.size;
  payload_push_u16(&p, 0xEEEE);
  payload_patch_rel16(&p, 0xEEEE, off16 + 2 + 20);
  ASSERT_EQ(*(uint16_t *)(p.data + off16), 20, "rel16 patch correct");

  // 64-bit rel
  size_t off64 = p.size;
  payload_push_u64(&p, 0x7777777777777777);
  payload_patch_rel64(&p, 0x7777777777777777, off64 + 8 + 100);
  ASSERT_EQ(*(uint64_t *)(p.data + off64), 100, "rel64 patch correct");

  payload_fini(&p);
}

void test_payload_variadic_macros(void) {
  payload_t p;
  payload_init(&p);

  PAYLOAD_PUSH_U32S(&p, 0x1, 0x2, 0x3);
  ASSERT_EQ(p.size, 12, "U32S macro size correct");
  uint32_t *d32 = (uint32_t *)p.data;
  ASSERT_EQ(d32[0], 1, "U32S item 0 correct");
  ASSERT_EQ(d32[2], 3, "U32S item 2 correct");

  size_t prev_size = p.size;
  PAYLOAD_PUSH_U64S(&p, 0xA, 0xB);
  ASSERT_EQ(p.size, prev_size + 16, "U64S macro size correct");
  uint64_t *d64 = (uint64_t *)(p.data + prev_size);
  ASSERT_EQ(d64[0], 0xA, "U64S item 0 correct");
  ASSERT_EQ(d64[1], 0xB, "U64S item 1 correct");

  payload_fini(&p);
}

void test_payload_push_growth(void) {
  payload_t p;
  payload_init(&p);
  uint8_t chunk[100];
  memset(chunk, 'A', 100);
  for (int i = 0; i < 5; i++) {
    payload_push(&p, chunk, 100);
  }
  ASSERT_TRUE(p.size == 500, "Growth size correct");
  ASSERT_TRUE(p.capacity >= 500, "Capacity expanded");
  payload_fini(&p);
}

int main(void) {
  test_patch_basic();
  test_patch_edge_cases();
  test_payload_push_ints();
  test_payload_patch_ints();
  test_payload_rel_all_sizes();
  test_payload_variadic_macros();
  test_payload_push_growth();
  return 0;
}
