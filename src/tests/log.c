#include <axium/log.h>
#include <axium/timeout.h>
#include <test_common.h>
#include <unistd.h>

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

  ASSERT_EXIT_FAIL(
      { log_error("This error message should be visible and exit"); },
      "log_error should exit with FAILURE");
}

void test_log_styles(void) {
  printf("--- All log styles (excluding fatal) ---\n");
  set_log_level(DEBUG);
  log_success("success (+)");
  log_failure("failure (-)");
  log_info("info (*)");
  log_warning("warning (!)");
  log_critical("critical (CRITICAL)");
  log_debug("debug (DEBUG)");
  log_status("status (x)");
  printf("\n"); // Status doesn't end with newline
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
  ASSERT_EXIT_FAIL(
      {
        FILE *f = fopen("/tmp/non-existent-file-axium-test", "r");
        if (!f) {
          log_exception("Expected failure to open file");
        }
      },
      "log_exception should exit with FAILURE");
}

void test_log_status(void) {
  printf("--- Testing log_status (visual check for overwriting) ---\n");
  set_log_level(INFO);

  for (int i = 0; i <= 5; i++) {
    if (i == 5)
      // Ensure the last update exceeds the throttle threshold (0.1s)
      usleep(110000);
    log_status("Counting: %d/5", i);
    if (i < 5)
      usleep(50000);
  }
  printf("\n(Previous updates should have been throttled/overwritten)\n");

  for (int i = 0; i <= 3; i++) {
    log_status("Interrupted count: %d", i);
    usleep(150000); // 0.15s, Slower than throttle rate
    if (i == 1) {
      log_info("Interruption at i=1");
    }
  }
  log_success("Status finished with success");
}

int main(void) {
  test_log_levels();
  test_log_filtering();
  test_log_styles();
  test_log_once();
  test_log_exception();
  test_log_status();
  return 0;
}
