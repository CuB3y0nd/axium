#include <axium/tubes/process.h>
#include <test_common.h>

int main(void) {
  // Test 1: Running a non-existent process
  char *const non_existent_argv[] = {"/bin/non-existent", NULL};
  tube *t1 = process(non_existent_argv, NULL);
  ASSERT_TRUE(t1 == NULL,
              "process() should return NULL for non-existent binary");

  if (t1 != NULL) {
    t_close(t1);
  }

  char *const cat_argv[] = {"/bin/cat", NULL};

  // Test 2: Using process_ext to capture ONLY stdin (original test logic)
  tube *t2 = process_ext(cat_argv, NULL, TUBE_STDIN);
  ASSERT_TRUE(t2 != NULL, "Create process for cat with TUBE_STDIN only");
  ASSERT_TRUE(t_in(t2) != -1, "Stdin should be captured");
  ASSERT_TRUE(t_out(t2) == -1, "Stdout should NOT be captured");
  ASSERT_TRUE(t_err(t2) == -1, "Stderr should NOT be captured");

  const char *test_input = "This is a test input sent to cat's stdin.\n";
  ssize_t written_bytes = write(t_in(t2), test_input, strlen(test_input));
  ASSERT_TRUE(written_bytes == (ssize_t)strlen(test_input),
              "Write all bytes to stdin");
  t_close(t2);

  // Test 3: Using process (default TUBE_ALL)
  tube *t3 = process(cat_argv, NULL);
  ASSERT_TRUE(t3 != NULL, "Create process for cat with default TUBE_ALL");
  ASSERT_TRUE(t_in(t3) != -1, "Stdin should be captured by default");
  ASSERT_TRUE(t_out(t3) != -1, "Stdout should be captured by default");
  ASSERT_TRUE(t_err(t3) != -1, "Stderr should be captured by default");
  t_close(t3);

  return 0;
}
