#include <axium/utils/fiddling.h>
#include <stdio.h>
#include <string.h>

void test_hexdump_basic(void) {
  printf("--- Basic Hexdump (Default Options) ---\n");
  const char *data = "Hello, World! This is a test of the hexdump utility.";
  hexdump(data, strlen(data), NULL);
  printf("\n");
}

void test_hexdump_colors(void) {
  printf("--- Hexdump with Special Bytes (Colors) ---\n");
  unsigned char data[32];
  for (int i = 0; i < 32; i++) {
    data[i] = i;
  }
  data[16] = 0xff;
  data[17] = 0x00;
  data[18] = 0x0a;
  data[19] = 0x20;

  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  hexdump(data, 32, &opts);
  printf("\n");
}

void test_hexdump_skip(void) {
  printf("--- Hexdump Skip Logic (*) ---\n");
  unsigned char data[64];
  memset(data, 'A', 64);
  /* Insert some unique data in the middle */
  memcpy(data + 32, "UNIQUE DATA HERE", 16);

  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  hexdump(data, 64, &opts);
  printf("\n");
}

void test_hexdump_no_skip(void) {
  printf("--- Hexdump No Skip (skip=false) ---\n");
  unsigned char data[48];
  memset(data, 'B', 48);

  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  opts.skip = false;
  hexdump(data, 48, &opts);
  printf("\n");
}

void test_hexdump_no_total(void) {
  printf("--- Hexdump No Total (total=false) ---\n");
  const char *data = "No total bytes line at the end.";
  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  opts.total = false;
  hexdump(data, strlen(data), &opts);
  printf("\n");
}

void test_hexdump_no_color(void) {
  printf("--- Hexdump No Color (color=false) ---\n");
  unsigned char data[] = {0x00, 0x41, 0x0a, 0xff, 0x01};
  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  opts.color = false;
  hexdump(data, sizeof(data), &opts);
  printf("\n");
}

void test_hexdump_custom_config(void) {
  printf(
      "--- Hexdump Custom Config (width=10, groupsize=5, begin=0x1234) ---\n");
  const char *data = "This is a test of custom width and grouping.";
  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  opts.width = 10;
  opts.groupsize = 5;
  opts.prefix = "[DEBUG] ";
  opts.begin = 0x1234;
  hexdump(data, strlen(data), &opts);
  printf("\n");
}

void test_hexdump_custom_theme(void) {
  printf("--- Hexdump Custom Theme (Inverted/Weird Colors) ---\n");
  unsigned char data[] = {0x00, 0x41, 0xff, 0x01, 0x31, 0x33, 0x33, 0x37};
  hexdump_options opts = HEXDUMP_DEFAULT_OPTIONS;
  /* Green for NULL, Red for 0xff, Purple for marker */
  opts.theme.null = "\x1b[32m";
  opts.theme.ff = "\x1b[31m";
  opts.theme.marker = "\x1b[35m";
  hexdump(data, sizeof(data), &opts);
  printf("\n");
}

void test_hexdump_edge_cases(void) {
  printf("--- Hexdump Edge Case: 1 Byte ---\n");
  hexdump("A", 1, NULL);

  printf("\n--- Hexdump Edge Case: Exact Width (16 bytes) ---\n");
  hexdump("1234567890ABCDEF", 16, NULL);

  printf("\n--- Hexdump Edge Case: Width + 1 (17 bytes) ---\n");
  hexdump("1234567890ABCDEF!", 17, NULL);
}

int main(void) {
  test_hexdump_basic();
  test_hexdump_colors();
  test_hexdump_skip();
  test_hexdump_no_skip();
  test_hexdump_no_total();
  test_hexdump_no_color();
  test_hexdump_custom_config();
  test_hexdump_custom_theme();
  test_hexdump_edge_cases();
  return 0;
}
