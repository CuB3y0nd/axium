#include "test_common.h"
#include <axium/log.h>

int main() {
  printf("--- Testing with INFO level (default) ---\n");
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

  printf("--- All log styles ---\n");
  set_log_level(DEBUG);
  log_success("success");
  log_failure("failure");
  log_info("info");
  log_warning("warning");
  log_error("error");
  log_critical("critical");
  log_debug("debug");
  log_status("status");

  printf("--- Testing log_info_once ---\n");
  for (int i = 0; i < 3; i++) {
    log_info_once("This should only appear ONCE (i=%d)", i);
  }

  printf("--- Testing log_exception ---\n");
  FILE *f = fopen("/tmp/non-existent-file", "r");
  if (!f) {
    log_exception("Failed to open file");
  }

  return 0;
}
