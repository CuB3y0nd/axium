#include <axium/axium.h>
#include <test_common.h>

#define ARRAY_SIZE 256

void test_fences(void) {
  log_info("Testing memory fences (mfence, lfence, sfence)...");
  mfence();
  lfence();
  sfence();
  log_success("Fences executed successfully.");
}

void test_timing_comparison(void) {
  uint64_t start, end;
  uint64_t overhead_std, overhead_fast;

  log_info("Comparing timing primitives overhead...");

  // Standard (CPUID)
  start = probe_start();
  end = probe_end();
  overhead_std = end - start;
  log_info("Standard instrumentation overhead: %lu cycles", overhead_std);

  // Might be Faster ? (LFENCE)
  start = probe_start_lfence();
  end = probe_end_lfence();
  overhead_fast = end - start;
  log_info("Fast instrumentation overhead:     %lu cycles", overhead_fast);

  if (overhead_fast < overhead_std) {
    log_success("Fast variant is indeed lighter.");
  }
}

void test_cache_logic_comprehensive(void) {
  uint64_t start, end;
  uint8_t buffer[1024];

  log_info("Testing Flush + Reload primitives with maccess...");

  // 1. Measured Miss with Standard probe
  clflush(buffer);
  start = probe_start();
  maccess(buffer);
  end = probe_end();
  log_info("Standard Buffer Access (Miss): %lu cycles", end - start);

  // 2. Measured Hit with Standard probe
  start = probe_start();
  maccess(buffer);
  end = probe_end();
  log_info("Standard Buffer Access (Hit):  %lu cycles", end - start);

  // 1. Measured Miss with lfence probe
  clflush(buffer);
  start = probe_start_lfence();
  maccess(buffer);
  end = probe_end_lfence();
  log_info("lfence Buffer Access (Miss): %lu cycles", end - start);

  // 2. Measured Hit with lfence probe
  start = probe_start_lfence();
  maccess(buffer);
  end = probe_end_lfence();
  log_info("lfence Buffer Access Fast (Hit):  %lu cycles", end - start);
}

int main(void) {
  set_log_level(DEBUG);
  test_fences();
  test_timing_comparison();
  test_cache_logic_comprehensive();
  return 0;
}
