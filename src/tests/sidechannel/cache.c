#include <axium/axium.h>
#include <test_common.h>

#define ARRAY_SIZE 256

void test_fences(void) {
  log_info("Testing memory fences (mfence, lfence, sfence)...");
  mfence();
  lfence();
  sfence();
}

void test_mixed_idx(void) {
  uint8_t mask = ARRAY_SIZE - 1;

  log_info("Testing MIXED_IDX mapping...");
  for (int i = 0; i < 5; i++) {
    uint32_t mixed = MIXED_IDX(i, mask);
    log_debug("Index %d maps to %u", i, mixed);
  }
}

void test_timing_primitives(void) {
  uint64_t t1, t2;

  log_info("Testing timing primitives and serializing overhead...");

  t1 = rdtsc();
  t2 = rdtscp();
  log_debug("Raw rdtsc -> rdtscp delta: %lu", t2 - t1);

  t1 = probe_start();
  t2 = probe_end();
  log_info("Instrumentation overhead (probe_start -> probe_end): %lu cycles",
           t2 - t1);

  t1 = probe_start_lfence();
  t2 = probe_end_lfence();
  log_info("Instrumentation overhead (probe_start_lfence -> probe_end_lfence): "
           "%lu cycles",
           t2 - t1);
}

void test_cache_logic(void) {
  uint64_t start, end;
  volatile int x = 0;

  log_info("Verifying Cache side-channel logic (Flush + Reload)...");

  // 1. Flush
  clflush((void *)&x);

  // 2. Measure Miss
  start = probe_start();
  maccess((void *)&x);
  end = probe_end();
  uint64_t miss_time = end - start;
  log_info("Access time after clflush (Miss): %lu cycles", miss_time);

  // 3. Measure Hit
  start = probe_start();
  maccess((void *)&x);
  end = probe_end();
  uint64_t hit_time = end - start;
  log_info("Access time after reload (Hit):  %lu cycles", hit_time);

  // Validation
  if (miss_time > hit_time) {
    log_success("Timing difference is consistent with cache behavior.");
  } else {
    log_warning("Timing difference is marginal; result might be noisy.");
  }
}

int main(void) {
  set_log_level(DEBUG);
  test_fences();
  test_mixed_idx();
  test_timing_primitives();
  test_cache_logic();
  return 0;
}
