#include <axium/utils/payload.h>
#include <test_common.h>

void test_patch_basic(void) {
  char buf[] = "Hello MARKER World";
  patch((uint8_t *)buf, strlen(buf), "MARKER", 6, "AXIUM!", 6);
  ASSERT_STREQ(buf, "Hello AXIUM! World", "Basic patch works");
}

void test_patch_single_byte(void) {
  char buf[] = "A B A C A";
  patch((uint8_t *)buf, strlen(buf), "A", 1, "X", 1);
  ASSERT_STREQ(buf, "X B X C X",
               "Single byte patch (multiple occurrences) works");
}

void test_patch_multiple(void) {
  char buf[] = "TAG1...TAG1...TAG1";
  patch((uint8_t *)buf, strlen(buf), "TAG1", 4, "DATA", 4);
  ASSERT_STREQ(buf, "DATA...DATA...DATA",
               "Multiple multi-byte occurrences works");
}

void test_patch_shorter_replacement(void) {
  char buf[] = "Before MARKER After";
  // MARKER is 6 bytes, REP is 3 bytes. Should be REP + 3 zeros.
  patch((uint8_t *)buf, strlen(buf), "MARKER", 6, "REP", 3);

  // buf should look like "Before REP\0\0\0 After"
  ASSERT_TRUE(memcmp(buf + 7, "REP\0\0\0", 6) == 0,
              "Shorter replacement zero-fills remaining space");
  ASSERT_TRUE(buf[7] == 'R' && buf[10] == '\0' && buf[13] == ' ',
              "Bytes at expected positions");
}

void test_patch_longer_replacement(void) {
  char buf[] = "Before MARKER After";
  // MARKER is 6 bytes, LONGER_REPLACEMENT is 18 bytes. Should truncate to 6.
  patch((uint8_t *)buf, strlen(buf), "MARKER", 6, "LONGER_REPLACEMENT", 18);
  ASSERT_STREQ(buf, "Before LONGER After",
               "Longer replacement is truncated to marker size");
}

void test_patch_not_found(void) {
  char buf[] = "No tag here";
  char orig[sizeof(buf)];
  memcpy(orig, buf, sizeof(buf));
  patch((uint8_t *)buf, strlen(buf), "MISSING", 7, "FOUNDIT", 7);
  ASSERT_STREQ(buf, orig, "Buffer remains unchanged if marker not found");
}

void test_patch_boundary(void) {
  char buf1[] = "MARKER at start";
  patch((uint8_t *)buf1, strlen(buf1), "MARKER", 6, "BEGIN!", 6);
  ASSERT_STREQ(buf1, "BEGIN! at start", "Patch at start of buffer");

  char buf2[] = "at end MARKER";
  patch((uint8_t *)buf2, strlen(buf2), "MARKER", 6, "FINISH", 6);
  ASSERT_STREQ(buf2, "at end FINISH", "Patch at end of buffer");
}

int main(void) {
  test_patch_basic();
  test_patch_single_byte();
  test_patch_multiple();
  test_patch_shorter_replacement();
  test_patch_longer_replacement();
  test_patch_not_found();
  test_patch_boundary();
  return 0;
}
