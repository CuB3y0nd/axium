#include <axium/axium.h>
#include <test_common.h>

void test_dynamic_line_size(void) {
  size_t step = cache_line_size();
  log_info("Hardware CLFLUSH line size: %zu bytes", step);
  ASSERT_TRUE(step > 0 && step % 8 == 0, "Valid cache line size");
}

void test_context_aware_calibration(void) {
  /* Test with NULL (local allocation) */
  uint64_t t0 = cache_calibrate_threshold(NULL);
  log_info("Local calibration threshold: %lu", t0);

  /* Test on stack */
  int x = 42;
  uint64_t t1 = cache_calibrate_threshold(&x);
  log_info("Stack calibration threshold: %lu", t1);

  /* Test on heap */
  void *p = malloc(4096);
  uint64_t t2 = cache_calibrate_threshold(p);
  log_info("Heap calibration threshold:  %lu", t2);

  ASSERT_TRUE(t0 > 0 && t1 > 0 && t2 > 0, "Calibration returns non-zero");
  free(p);
}

void test_gap_refinement_logic(void) {
  cache_report_t report;

  log_info("Scenario 1: Noise near threshold should be filtered.");
  /* Winner=60, Noise=190. Threshold=200. Gap=130. Effective becomes ~125. */
  uint64_t timings1[] = {190, 60, 195, 205, 400};
  cache_analyze(&report, timings1, 5, 200);

  ASSERT_EQ(report.winner_idx, 1, "Correct winner identified");
  ASSERT_EQ(report.hits_count, 1, "Noise successfully filtered");
  log_success("Noise filtering OK.");

  log_info("Scenario 2: Adaptive winner (exceeds threshold but large gap).");
  /* Winner=210, RunnerUp=400. Threshold=180. Gap=190. Effective becomes ~305.
   */
  uint64_t timings2[] = {400, 410, 210, 420, 395};
  cache_analyze(&report, timings2, 5, 180);

  ASSERT_EQ(report.winner_idx, 2, "Adaptive winner identified");
  ASSERT_EQ(report.hits_count, 1, "Adaptive hit count correct");
  log_success("Adaptive detection OK.");
}

void test_range_flushing_dynamic(void) {
  size_t size = 8192;
  uint8_t *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memset(ptr, 0xCC, size);

  log_info("Testing range flush on 8KB mapping...");
  cache_flush_range(ptr, size);

  /* Measure the first and middle byte to ensure they are both cold */
  uint64_t s = probe_start();
  maccess(ptr);
  uint64_t e = probe_end();
  log_info("Cold access (start): %lu", e - s);
  ASSERT_TRUE(e - s > 100, "Start of range is cold");

  s = probe_start();
  maccess(ptr + 4096);
  e = probe_end();
  log_info("Cold access (mid):   %lu", e - s);
  ASSERT_TRUE(e - s > 100, "Middle of range is cold");

  munmap(ptr, size);
}

int main(void) {
  test_dynamic_line_size();
  test_context_aware_calibration();
  test_gap_refinement_logic();
  test_range_flushing_dynamic();
  return 0;
}
