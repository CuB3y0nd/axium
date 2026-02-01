#include <axium/log.h>
#include <axium/sidechannel/monitor.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
  const cache_watch_report_t *report;
  char filename[512];
} _watch_sig_ctx;

static void _watch_internal_sigint_handler(int sig) {
  (void)sig;
  puts(""); /* New line after potentially many hit logs */
  log_info("Interrupted. Exporting watch report...");

  if (cache_export_watch_report(_watch_sig_ctx.report,
                                _watch_sig_ctx.filename) == 0) {
    exit(0);
  } else {
    log_error("Failed to save report on exit.");
  }
}

void cache_watch_install_handler(const cache_watch_report_t *report,
                                 const char *filename) {
  _watch_sig_ctx.report = report;
  strncpy(_watch_sig_ctx.filename, filename,
          sizeof(_watch_sig_ctx.filename) - 1);
  signal(SIGINT, _watch_internal_sigint_handler);
}

cache_watch_config cache_watch_config_init(uint64_t threshold, size_t count,
                                           size_t stride, int wait_cycles) {
  cache_watch_config config = {
      .threshold = threshold,
      .count = count,
      .stride = stride,
      .wait_cycles = wait_cycles,
  };
  return config;
}

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

  if (!callback) {
    callback = cache_watch_reporter;
  }

  while (1) {
    log_info_once("Watching... (Press Ctrl+C to exit)");

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
