#include <axium/tubes/process.h>
#include <stdio.h>
#include <string.h>

#define ASSERT_TRUE(condition, message)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: %s at %s:%d - %s\n", #condition,      \
              __FILE__, __LINE__, message);                                    \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main() {
  printf("Running test_process.c\n");

  char *const cat_argv[] = {"/bin/cat", NULL};
  tube *t = process(cat_argv, NULL);
  ASSERT_TRUE(t != NULL, "Failed to create process for cat");

  const char *test_input = "This is a test.\n";
  ssize_t written_bytes = write(t_in(t), test_input, strlen(test_input));
  ASSERT_TRUE(written_bytes == (ssize_t)strlen(test_input),
              "Failed to write all bytes to stdin");

  // Close stdin to signal EOF to cat
  t_cin(t);

  char buffer[1024];
  ssize_t read_bytes;
  char received_output[1024] = {0};
  size_t total_read = 0;

  while ((read_bytes = read(t_out(t), buffer, sizeof(buffer) - 1)) > 0) {
    if (total_read + (size_t)read_bytes >= sizeof(received_output)) {
      fprintf(stderr, "Buffer too small for received output.\n");
      break;
    }
    memcpy(received_output + total_read, buffer, read_bytes);
    total_read += read_bytes;
  }
  received_output[total_read] = '\0';

  ASSERT_TRUE(strcmp(test_input, received_output) == 0,
              "Received output does not match sent input");

  t_close(t);

  printf("Test passed: Process communication successful.\n");

  return 0;
}
