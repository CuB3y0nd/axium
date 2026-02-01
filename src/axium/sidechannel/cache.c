#include <axium/log.h>
#include <axium/sidechannel/cache.h>
#include <stdlib.h>

uint64_t cache_calibrate_threshold(const void *target) {
  uint64_t hit_min = (uint64_t)-1;
  uint64_t miss_min = (uint64_t)-1;
  const int rounds = 1000;
  uint8_t *cal_buf = NULL;
  bool is_local = false;

  /* Select memory context for calibration */
  if (target) {
    /* Align to page boundary to stay within safe memory */
    cal_buf = (uint8_t *)((uintptr_t)target & ~0xFFF);
  } else {
    /* Fallback to heap allocation to avoid stack noise */
    cal_buf = (uint8_t *)malloc(4096);
    if (!cal_buf)
      log_error("malloc failed");
    for (int i = 0; i < 4096; i++)
      cal_buf[i] = (uint8_t)i;
    is_local = true;
  }

  for (int i = 0; i < rounds; i++) {
    /* Pseudo-random jumps to defeat hardware prefetchers */
    size_t offset = (i * 167 + 13) & 0xF80;

    /* Measure Cache Miss baseline */
    clflush(&cal_buf[offset]);
    uint64_t start = probe_start();
    maccess(&cal_buf[offset]);
    uint64_t end = probe_end();
    uint64_t duration = end - start;
    if (duration < miss_min && duration > 0)
      miss_min = duration;

    /* Measure Cache Hit baseline */
    start = probe_start();
    maccess(&cal_buf[offset]);
    end = probe_end();
    duration = end - start;
    if (duration < hit_min && duration > 0)
      hit_min = duration;
  }

  if (is_local)
    free(cal_buf);

  log_debug("Calibration complete: Min Hit = %lu, Min Miss = %lu", hit_min,
            miss_min);

  /* Ensure sanity: Miss must be significantly slower than Hit */
  if (miss_min < hit_min + 50)
    miss_min = hit_min + 200;

  /* Balanced threshold for general environments */
  return (hit_min + miss_min) / 2;
}
