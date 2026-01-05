#include <axium/tubes/process.h>
#include <test_common.h>

void test_process_non_existent(void) {
  printf("--- Testing non-existent process ---\n");
  char *const argv[] = {"/bin/non-existent-binary-axium", NULL};
  tube *t = process(argv, NULL);
  ASSERT_TRUE(t == NULL, "process() returns NULL for non-existent binary");
}

void test_process_ext_partial_capture(void) {
  printf("--- Testing process_ext with TUBE_STDIN only ---\n");
  char *const argv[] = {"/bin/cat", NULL};
  tube *t = process_ext(argv, NULL, TUBE_STDIN);

  ASSERT_TRUE(t != NULL, "Spawn cat with STDIN only");
  ASSERT_TRUE(t_in(t) != -1, "Stdin is captured");
  ASSERT_TRUE(t_out(t) == -1, "Stdout is NOT captured");
  ASSERT_TRUE(t_err(t) == -1, "Stderr is NOT captured");

  const char *msg = "test\n";
  ssize_t n = write(t_in(t), msg, 5);
  ASSERT_TRUE(n == 5, "Write to captured stdin succeeds");

  t_close(t);
}

void test_process_default_all_capture(void) {
  printf("--- Testing process() with default TUBE_ALL ---\n");
  char *const argv[] = {"/bin/cat", NULL};
  tube *t = process(argv, NULL);

  ASSERT_TRUE(t != NULL, "Spawn cat with default flags");
  ASSERT_TRUE(t_in(t) != -1, "Stdin captured");
  ASSERT_TRUE(t_out(t) != -1, "Stdout captured");
  ASSERT_TRUE(t_err(t) != -1, "Stderr captured");

  t_close(t);
}

int main(void) {
  test_process_non_existent();
  test_process_ext_partial_capture();
  test_process_default_all_capture();
  return 0;
}
