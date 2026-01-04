#include <axium/utils/payload.h>
#include <test_common.h>

int main(void) {
  char payload[] = "The PID is XXXXXXXXXX and the path is /proc/XXXXXXXXXX/mem";
  const char *marker = "XXXXXXXXXX";
  const char *replacement = "1234";

  patch((uint8_t *)payload, sizeof(payload), marker, strlen(marker),
        replacement, strlen(replacement));

  ASSERT_STREQ(payload, "The PID is 1234", "Payload patched correctly");

  ASSERT_TRUE(payload[11] == '1', "Byte at offset 11 is '1'");
  ASSERT_TRUE(payload[15] == '\0', "Byte at offset 15 is null terminator");
  ASSERT_TRUE(payload[16] == '\0', "Byte at offset 16 is null terminator");

  char payload2[] = "Hello MARKER";
  patch((uint8_t *)payload2, sizeof(payload2), "MARKER", 6, "WORLD!", 6);
  ASSERT_STREQ(payload2, "Hello WORLD!", "Exact size replacement");

  return 0;
}
