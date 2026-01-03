#include <axium/tubes/process.h>
#include <test_common.h>

int main(void) {
  char *const non_existent_argv[] = {"/bin/non-existent", NULL};
  tube *t1 = process(non_existent_argv, NULL);
  ASSERT_TRUE(t1 == NULL,
              "process() should return NULL for non-existent binary");

  if (t1 != NULL) {
    t_close(t1);
  }

  char *const cat_argv[] = {"/bin/cat", NULL};
  tube *t2 = process(cat_argv, NULL);
  ASSERT_TRUE(t2 != NULL, "Create process for cat");

  const char *test_input = "This is a test.\n";
  ssize_t written_bytes = write(t_in(t2), test_input, strlen(test_input));
  ASSERT_TRUE(written_bytes == (ssize_t)strlen(test_input),
              "Write all bytes to stdin");

  // Close stdin to signal EOF to cat
  t_cin(t2);

  char buffer[1024];
  ssize_t read_bytes;
  char received_output[1024] = {0};
  size_t total_read = 0;

  while ((read_bytes = read(t_out(t2), buffer, sizeof(buffer) - 1)) > 0) {
    if (total_read + (size_t)read_bytes >= sizeof(received_output)) {
      fprintf(stderr, "Buffer too small for received output.\n");
      break;
    }
    memcpy(received_output + total_read, buffer, read_bytes);
    total_read += read_bytes;
  }
  received_output[total_read] = '\0';

  ASSERT_STREQ(received_output, test_input,
               "Received output matches sent input");

  t_close(t2);

  return 0;
}
