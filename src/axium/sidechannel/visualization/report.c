#include <axium/log.h>
#include <axium/sidechannel/cache.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cache_report(const cache_report_t *report) {
  if (!report)
    return;

  /* Calculate adaptive widths for formatting */
  int idx_width = 1;
  size_t temp_count = report->count;
  while (temp_count >= 10) {
    temp_count /= 10;
    idx_width++;
  }

  uint64_t max_val = 0;
  for (size_t i = 0; i < report->count; i++) {
    if (report->timings[i] > max_val)
      max_val = report->timings[i];
  }
  int val_width = 1;
  uint64_t temp_val = max_val;
  while (temp_val >= 10) {
    temp_val /= 10;
    val_width++;
  }
  if (val_width < 4)
    val_width = 4;

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

    log_info("Idx %*zu: [%s] %*lu cycles %s", idx_width, i, bar, val_width, t,
             marker);
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

void cache_watch_reporter(size_t index, uint64_t cycles, void *user_data) {
  if (!user_data) {
    log_info("Hit Index %-3zu | Latency: %lu", index, cycles);
    return;
  }

  cache_watch_report_t *report = (cache_watch_report_t *)user_data;

  if (__builtin_expect(!report->hit_counts, 0)) {
    log_info("Hit Index %-3zu | Latency: %-3lu", index, cycles);
    return;
  }

  if (__builtin_expect(index >= report->count, 0)) {
    log_warning("Hit Index %zu out of bounds (count: %zu)", index,
                report->count);
    return;
  }

  report->hit_counts[index]++;
  report->total_hits++;

  log_info("Hit Index %-3zu | Latency: %-3lu | Index Hits: %-5lu | Total: %lu",
           index, cycles, report->hit_counts[index], report->total_hits);
}

void cache_watch_report_init(cache_watch_report_t *report, uint64_t *hit_counts,
                             size_t count, uint64_t threshold) {
  if (!report || !hit_counts)
    return;

  report->hit_counts = hit_counts;
  report->count = count;
  report->threshold = threshold;
  report->total_hits = 0;

  for (size_t i = 0; i < count; i++) {
    report->hit_counts[i] = 0;
  }
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

  /* Global identification of winner and runner-up */
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

  /* Refine threshold using Gap analysis if a clear jump is found */
  if (report->gap >= 50) {
    /* Use the midpoint between top two candidates as the refined cutoff */
    report->effective_threshold = report->winner_val + (report->gap / 2);
  }

  /* Filter results based on the effective threshold */
  for (size_t i = 0; i < count; i++) {
    if (timings[i] <= report->effective_threshold) {
      report->hits_count++;
    }
  }

  /* Ensure adaptive logic flags the winner if the gap is decisive */
  if (report->hits_count == 0 && report->gap >= 50 &&
      report->winner_idx != -1) {
    report->hits_count = 1;
  } else if (report->winner_val > report->effective_threshold) {
    /* No true winner found if the best candidate is still too slow */
    report->winner_idx = -1;
    report->hits_count = 0;
  }
}

int cache_export_report(const cache_report_t *report, const char *filename) {
  if (!report || !report->timings || !filename)
    return -1;

  FILE *f = fopen(filename, "w");
  if (!f) {
    log_error("Failed to open file for export: %s", filename);
  }

  fprintf(f, "{\n");
  fprintf(f, "  \"threshold\": %lu,\n", report->threshold);
  fprintf(f, "  \"effective_threshold\": %lu,\n", report->effective_threshold);
  fprintf(f, "  \"winner_idx\": %d,\n", report->winner_idx);
  fprintf(f, "  \"winner_val\": %lu,\n", report->winner_val);
  fprintf(f, "  \"gap\": %lu,\n", report->gap);
  fprintf(f, "  \"hits_count\": %zu,\n", report->hits_count);
  fprintf(f, "  \"count\": %zu,\n", report->count);
  fprintf(f, "  \"timings\": [");

  for (size_t i = 0; i < report->count; i++) {
    fprintf(f, "%lu%s", report->timings[i],
            (i == report->count - 1) ? "" : ", ");
  }

  log_success("Report saved to %s", filename);

  fprintf(f, "]\n}\n");
  fclose(f);
  return 0;
}

int cache_export_watch_report(const cache_watch_report_t *report,
                              const char *filename) {
  if (!report || !filename)
    return -1;

  FILE *f = fopen(filename, "w");
  if (!f) {
    log_error("Failed to open file for export: %s", filename);
  }

  fprintf(f, "{\n");
  fprintf(f, "  \"type\": \"watch\",\n");
  fprintf(f, "  \"count\": %zu,\n", report->count);
  fprintf(f, "  \"threshold\": %lu,\n", report->threshold);
  fprintf(f, "  \"total_hits\": %lu,\n", report->total_hits);
  fprintf(f, "  \"hit_counts\": [");
  for (size_t i = 0; i < report->count; i++) {
    fprintf(f, "%lu%s", report->hit_counts[i],
            (i == report->count - 1) ? "" : ", ");
  }
  fprintf(f, "]\n");
  fprintf(f, "}\n");

  log_success("Report saved to %s", filename);

  fclose(f);
  return 0;
}

void cache_view_report(const char *filename) {
  if (!filename)
    log_error("filename cannot be NULL");

  char cmd[2048];

  /* Try to find the visualization tool in common locations */
  const char *tool_paths[] = {"tools/cache_vis.html", "../tools/cache_vis.html",
                              "../../tools/cache_vis.html"};

  const char *found_tool = NULL;
  for (size_t i = 0; i < sizeof(tool_paths) / sizeof(tool_paths[0]); i++) {
    if (access(tool_paths[i], F_OK) == 0) {
      found_tool = tool_paths[i];
      break;
    }
  }

  if (!found_tool) {
    log_error("Could not find tools/cache_vis.html visualization tool.");
  }

  /* Get absolute path for the report */
  char abs_report[512];
  if (!realpath(filename, abs_report)) {
    log_error("Failed to resolve absolute path for report: %s", filename);
  }

  /* Get absolute path for the tool */
  char abs_tool[512];
  if (!realpath(found_tool, abs_tool)) {
    log_error("Failed to resolve absolute path for tool: %s", found_tool);
  }

  log_status("Opening visualization tool...");

#if defined(__linux__)
  snprintf(cmd, sizeof(cmd),
           "xdg-open \"file://%s?report=%s\" > /dev/null 2>&1 &", abs_tool,
           abs_report);
#elif defined(__APPLE__)
  snprintf(cmd, sizeof(cmd), "open \"file://%s?report=%s\" &", abs_tool,
           abs_report);
#else
  /* Fallback or Windows */
  snprintf(cmd, sizeof(cmd), "start \"\" \"file://%s?report=%s\"", abs_tool,
           abs_report);
#endif

  if (system(cmd) != 0) {
    log_error("Failed to execute open command.");
  }
}
