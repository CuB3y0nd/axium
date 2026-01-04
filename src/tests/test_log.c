#include <axium/log.h>
#include <test_common.h>

void test_log_levels(void) {
  printf("--- Testing log levels and get/set ---\n");
  set_log_level(WARNING);
  ASSERT_TRUE(get_log_level() == WARNING,
              "get_log_level returns WARNING after set");

  set_log_level(DEBUG);
  ASSERT_TRUE(get_log_level() == DEBUG,
              "get_log_level returns DEBUG after set");
}

void test_log_filtering(void) {
  printf("--- Testing with INFO level (default) ---\n");
  set_log_level(INFO);
  log_debug("This debug message should NOT be visible");
  log_success("This success message should be visible");

  printf("--- Testing with DEBUG level ---\n");
  set_log_level(DEBUG);
  log_debug("This debug message should be visible");

  printf("--- Testing with WARNING level ---\n");
  set_log_level(WARNING);
  log_info("This info message should NOT be visible");
  log_warning("This warning message should be visible");
  log_error("This error message should be visible");
}

void test_log_styles(void) {
  printf("--- All log styles ---\n");
  set_log_level(DEBUG);
  log_success("success (+)");
  log_failure("failure (-)");
  log_info("info (*)");
  log_warning("warning (!)");
  log_error("error (ERROR)");
  log_critical("critical (CRITICAL)");
  log_debug("debug (DEBUG)");
  log_status("status (x)");
}

void test_log_once(void) {
  printf("--- Testing log_info_once ---\n");
  set_log_level(INFO);
  for (int i = 0; i < 3; i++) {
    log_info_once("This should only appear ONCE (i=%d)", i);
  }

  printf("--- Testing log_warning_once ---\n");
  for (int i = 0; i < 3; i++) {
    log_warning_once("This warning should only appear ONCE (i=%d)", i);
  }
}

void test_log_exception(void) {
  printf("--- Testing log_exception ---\n");
  FILE *f = fopen("/tmp/non-existent-file-axium-test", "r");
  if (!f) {
    log_exception("Expected failure to open file");
  }
}

int main(void) {
  test_log_levels();
  test_log_filtering();
  test_log_styles();
  test_log_once();
  test_log_exception();

  printf(
      "All log tests completed (visual verification recommended for colors)\n");
  return 0;
}
