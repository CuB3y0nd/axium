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
  ASSERT_TRUE(t_out(t2) == -1, "Stdout should not be captured");
  ASSERT_TRUE(t_err(t2) == -1, "Stderr should not be captured");

  const char *test_input = "This is a test input sent to cat's stdin.\n";
  ssize_t written_bytes = write(t_in(t2), test_input, strlen(test_input));
  ASSERT_TRUE(written_bytes == (ssize_t)strlen(test_input),
              "Write all bytes to stdin");

  // Close stdin to signal EOF to cat
  t_cin(t2);
  t_close(t2);

  return 0;
}
