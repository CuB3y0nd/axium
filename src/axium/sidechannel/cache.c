#include <axium/log.h>
#include <axium/sidechannel/cache.h>
#include <stdbool.h>
#include <stdio.h>
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

void cache_analyze(cache_report_t *report, uint64_t *timings, size_t count,
                   uint64_t threshold) {
  if (!report || !timings || count == 0)
    return;

  report->timings = timings;
  report->count = count;
  report->threshold = threshold;
  report->effective_threshold = threshold;
  report->winner_idx = -1;
  report->winner_val = (uint64_t)-1;
  report->gap = 0;
  report->hits_count = 0;

  uint64_t runner_up_val = (uint64_t)-1;

  /* 1. Global identification of winner and runner-up */
  for (size_t i = 0; i < count; i++) {
    uint64_t t = timings[i];
    if (t < report->winner_val) {
      runner_up_val = report->winner_val;
      report->winner_val = t;
      report->winner_idx = (int)i;
    } else if (t < runner_up_val) {
      runner_up_val = t;
    }
  }

  if (report->winner_idx != -1 && runner_up_val != (uint64_t)-1) {
    report->gap = runner_up_val - report->winner_val;
  }

  /* 2. Refine threshold using Gap analysis if a clear jump is found */
  if (report->gap >= 50) {
    /* Use the midpoint between top two candidates as the refined cutoff */
    report->effective_threshold = report->winner_val + (report->gap / 2);
  }

  /* 3. Filter results based on the effective threshold */
  for (size_t i = 0; i < count; i++) {
    if (timings[i] <= report->effective_threshold) {
      report->hits_count++;
    }
  }

  /* 4. Ensure adaptive logic flags the winner if the gap is decisive */
  if (report->hits_count == 0 && report->gap >= 50 &&
      report->winner_idx != -1) {
    report->hits_count = 1;
  } else if (report->winner_val > report->effective_threshold) {
    /* No true winner found if the best candidate is still too slow */
    report->winner_idx = -1;
    report->hits_count = 0;
  }
}

void cache_report(const cache_report_t *report) {
  if (!report)
    return;

  log_info("--- Cache Side-Channel Report ---");
  log_info("Threshold: %lu (Effective: %lu) | Hits: %zu | Gap: %lu",
           report->threshold, report->effective_threshold, report->hits_count,
           report->gap);

  for (size_t i = 0; i < report->count; i++) {
    uint64_t t = report->timings[i];
    bool is_hit = t <= report->effective_threshold;
    bool is_winner = ((int)i == report->winner_idx);

    const char *marker = "";
    if (is_winner) {
      marker = (t <= report->threshold) ? " [WINNER]" : " [ADAPTIVE WINNER]";
    } else if (is_hit) {
      marker = " [HIT]";
    }

    char bar[21];
    size_t bar_len = (t > 500) ? 20 : (t / 25);
    if (bar_len > 20)
      bar_len = 20;
    for (size_t j = 0; j < 20; j++)
      bar[j] = (j < bar_len) ? '#' : ' ';
    bar[20] = '\0';

    log_info("Idx %2zu: [%s] %4lu cycles %s", i, bar, t, marker);
  }

  if (report->winner_idx != -1) {
    if (report->winner_val > report->threshold) {
      log_status("Adaptive logic: Winner (%lu) > Threshold (%lu), but Gap "
                 "(%lu) provides high confidence.",
                 report->winner_val, report->threshold, report->gap);
    }

    if (report->gap >= 50) {
      log_success("High confidence candidate: Index %d (Gap: %lu)",
                  report->winner_idx, report->gap);
    } else {
      log_warning("Low confidence candidate: Index %d (Gap: %lu)",
                  report->winner_idx, report->gap);
    }
  } else {
    log_failure("Detection failed: No results reached the required latency.");
  }
}
