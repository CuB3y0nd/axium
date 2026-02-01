#include <axium/log.h>
#include <axium/sidechannel/monitor.h>

int cache_audit(const void *target, uint64_t threshold) {
  clflush(target);
  mfence();
  uint64_t start = probe_start();
  maccess(target);
  uint64_t end = probe_end();
  uint64_t delta = end - start;

  if (delta < threshold) {
    return -1;
  }
  return 0;
}

void cache_watch(const void *base, const cache_watch_config *config,
                 cache_hit_cb callback, void *user_data) {
  const char *restrict ptr = (const char *)base;
  const size_t count = config->count;
  const size_t stride = config->stride;
  const uint64_t threshold = config->threshold;
  const int wait_cycles = config->wait_cycles;

  while (1) {
    for (size_t i = 0; i < count; i++) {
      clflush(ptr + (i * stride));
    }
    mfence();

    /* Small delay to open the vulnerability window. */
    for (volatile int delay = 0; delay < wait_cycles; delay++)
      __asm__ __volatile__("pause");

    for (size_t i = 0; i < count; i++) {
      size_t idx = MIXED_IDX(i, count - 1);
      const char *target_line = ptr + (idx * stride);

      uint64_t start = probe_start();
      maccess(target_line);
      uint64_t end = probe_end();
      uint64_t delta = end - start;

      if (__builtin_expect(delta < threshold, 0)) {
        callback(idx, delta, user_data);
        /* Immediate re-flush to avoid double-counting the same access.
         * No mfence needed here as the outer loop handles synchronization
         * and clflush is ordered with respect to the same address. */
        clflush(target_line);
      }
    }
  }
}
